#include "http/h2/H2Session.hpp"

#include "http/h2/H2ResponseHeaders.hpp"

#include <stdexcept>
#include <utility>

namespace {

std::vector<nghttp2_nv> name_values(H2HeaderFields& fields) {
   std::vector<nghttp2_nv> nvs;
   nvs.reserve(fields.size());
   for(auto& field : fields) {
      nghttp2_nv nv{};
      nv.name = reinterpret_cast<std::uint8_t*>(field.first.data());
      nv.namelen = field.first.size();
      nv.value = reinterpret_cast<std::uint8_t*>(field.second.data());
      nv.valuelen = field.second.size();
      nv.flags = NGHTTP2_NV_FLAG_NONE;
      nvs.push_back(nv);
   }
   return nvs;
}

}  // namespace

/// Handed to the application for exactly one request.
class H2Session::Responder final : public IH2Responder {
   public:
      Responder(H2Session& session, int32_t stream_id) : session_(session), stream_id_(stream_id) {}

      void submit(int status, std::vector<H2Header> headers, std::string body) override {
         submitted_ = true;
         session_.submit_response(stream_id_, status, std::move(headers), std::move(body));
      }

      bool submitted() const override { return submitted_; }

   private:
      H2Session& session_;
      int32_t stream_id_;
      bool submitted_ = false;
};

H2Session::H2Session(IH2Application* application) : application_(application) {
   if(application_ == nullptr) {
      throw std::invalid_argument("HTTP/2 server session requires an application");
   }

   nghttp2_session_callbacks* callbacks = nullptr;
   if(nghttp2_session_callbacks_new(&callbacks) != 0) {
      throw std::runtime_error("nghttp2_session_callbacks_new failed");
   }

   nghttp2_session_callbacks_set_on_begin_headers_callback(callbacks, &H2Session::on_begin_headers);
   nghttp2_session_callbacks_set_on_header_callback(callbacks, &H2Session::on_header);
   nghttp2_session_callbacks_set_on_data_chunk_recv_callback(callbacks, &H2Session::on_data_chunk);
   nghttp2_session_callbacks_set_on_frame_recv_callback(callbacks, &H2Session::on_frame_recv);
   nghttp2_session_callbacks_set_on_stream_close_callback(callbacks, &H2Session::on_stream_close);

   const int created = nghttp2_session_server_new(&session_, callbacks, this);
   nghttp2_session_callbacks_del(callbacks);
   if(created != 0) {
      throw std::runtime_error(nghttp2_strerror(created));
   }

   // The connection preface must start with SETTINGS. Leave the values at
   // nghttp2's defaults: this server does not offer extended CONNECT.
   const int settings = nghttp2_submit_settings(session_, NGHTTP2_FLAG_NONE, nullptr, 0);
   if(settings != 0) {
      nghttp2_session_del(session_);
      session_ = nullptr;
      throw std::runtime_error(nghttp2_strerror(settings));
   }
}

H2Session::~H2Session() {
   if(session_ != nullptr) {
      nghttp2_session_del(session_);
   }
}

void H2Session::submit_response(int32_t stream_id, int status, std::vector<H2Header> headers, std::string body) {
   const std::size_t body_size = body.size();
   H2HeaderFields fields = build_response_fields(status, headers, body_size);
   const std::vector<nghttp2_nv> nvs = name_values(fields);

   nghttp2_data_provider2 provider{};
   const nghttp2_data_provider2* data = nullptr;
   if(body_size > 0) {
      provider.source.ptr = bodies_.add(stream_id, std::move(body));
      provider.read_callback = &H2Session::on_read_body;
      data = &provider;
   }

   const int rv = nghttp2_submit_response2(session_, stream_id, nvs.data(), nvs.size(), data);
   if(rv != 0) {
      bodies_.remove(stream_id);
      throw std::runtime_error(nghttp2_strerror(rv));
   }
}

void H2Session::consume(const std::uint8_t* data, std::size_t len) {
   const nghttp2_ssize rv = nghttp2_session_mem_recv2(session_, data, len);
   if(app_error_) {
      std::rethrow_exception(app_error_);
   }
   if(rv < 0) {
      throw std::runtime_error(nghttp2_strerror(static_cast<int>(rv)));
   }
}

std::vector<std::uint8_t> H2Session::next_outbound() {
   const std::uint8_t* data = nullptr;
   const nghttp2_ssize n = nghttp2_session_mem_send2(session_, &data);
   if(n < 0) {
      throw std::runtime_error(nghttp2_strerror(static_cast<int>(n)));
   }
   if(n == 0 || data == nullptr) {
      return {};
   }
   return std::vector<std::uint8_t>(data, data + n);
}

bool H2Session::wants_read() const {
   return nghttp2_session_want_read(session_) != 0;
}

bool H2Session::wants_write() const {
   return nghttp2_session_want_write(session_) != 0;
}

void H2Session::complete_stream(int32_t stream_id) {
   std::optional<H2Request> request = requests_.complete(stream_id);
   if(!request) {
      return;
   }

   Responder responder(*this, stream_id);
   try {
      application_->on_request(*request, responder);
      if(!responder.submitted()) {
         responder.submit(500, {{"content-type", "text/plain"}}, "missing response");
      }
   } catch(...) {
      app_error_ = std::current_exception();
   }
}

int H2Session::on_begin_headers(nghttp2_session*, const nghttp2_frame* frame, void* user_data) {
   if(frame->hd.type != NGHTTP2_HEADERS) {
      return 0;
   }
   static_cast<H2Session*>(user_data)->requests_.begin_stream(frame->hd.stream_id);
   return 0;
}

int H2Session::on_header(nghttp2_session*,
                         const nghttp2_frame* frame,
                         const std::uint8_t* name_bytes,
                         std::size_t name_len,
                         const std::uint8_t* value_bytes,
                         std::size_t value_len,
                         std::uint8_t,
                         void* user_data) {
   if(frame->hd.type != NGHTTP2_HEADERS) {
      return 0;
   }

   static_cast<H2Session*>(user_data)->requests_.add_header(
      frame->hd.stream_id,
      std::string_view(reinterpret_cast<const char*>(name_bytes), name_len),
      std::string_view(reinterpret_cast<const char*>(value_bytes), value_len));
   return 0;
}

int H2Session::on_data_chunk(
   nghttp2_session*, std::uint8_t, int32_t stream_id, const std::uint8_t* data, std::size_t len, void* user_data) {
   static_cast<H2Session*>(user_data)->requests_.add_body(stream_id, reinterpret_cast<const char*>(data), len);
   return 0;
}

int H2Session::on_frame_recv(nghttp2_session*, const nghttp2_frame* frame, void* user_data) {
   auto* self = static_cast<H2Session*>(user_data);
   if(frame->hd.stream_id == 0 || !self->requests_.has_stream(frame->hd.stream_id)) {
      return 0;
   }

   if((frame->hd.type != NGHTTP2_HEADERS && frame->hd.type != NGHTTP2_DATA) ||
      (frame->hd.flags & NGHTTP2_FLAG_END_STREAM) == 0) {
      return 0;
   }

   self->complete_stream(frame->hd.stream_id);
   if(self->app_error_) {
      return NGHTTP2_ERR_CALLBACK_FAILURE;
   }
   return 0;
}

int H2Session::on_stream_close(nghttp2_session*, int32_t stream_id, std::uint32_t, void* user_data) {
   auto* self = static_cast<H2Session*>(user_data);
   self->requests_.discard(stream_id);
   self->bodies_.remove(stream_id);
   return 0;
}

nghttp2_ssize H2Session::on_read_body(nghttp2_session*,
                                      int32_t,
                                      std::uint8_t* buf,
                                      std::size_t length,
                                      std::uint32_t* data_flags,
                                      nghttp2_data_source* source,
                                      void*) {
   auto* body = static_cast<H2OutboundBodies::Body*>(source->ptr);
   bool eof = false;
   const std::size_t n = body->read(buf, length, eof);
   if(eof) {
      *data_flags |= NGHTTP2_DATA_FLAG_EOF;
   }
   return static_cast<nghttp2_ssize>(n);
}
