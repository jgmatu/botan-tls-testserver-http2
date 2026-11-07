#pragma once

#include "http/h2/H2OutboundBodies.hpp"
#include "http/h2/H2RequestAssembler.hpp"
#include "http/h2/IH2Application.hpp"
#include "http/h2/IH2Engine.hpp"

#include <nghttp2/nghttp2.h>

#include <cstddef>
#include <cstdint>
#include <exception>
#include <string>
#include <vector>

/**
 * One HTTP/2 connection, server role only, on top of nghttp2. It owns the
 * nghttp2 session and nothing else: bytes in, bytes out, and one completed
 * request at a time handed to the application. No I/O happens here (see
 * H2ConnectionRunner), there is no HTTP/1.1 parser and no upgrade from HTTP/1.1.
 */
class H2Session final : public IH2Engine {
   public:
      explicit H2Session(IH2Application* application);
      ~H2Session() override;

      H2Session(const H2Session&) = delete;
      H2Session& operator=(const H2Session&) = delete;

      void consume(const std::uint8_t* data, std::size_t len) override;
      std::vector<std::uint8_t> next_outbound() override;
      bool wants_read() const override;
      bool wants_write() const override;

   private:
      class Responder;
      friend class Responder;

      void submit_response(int32_t stream_id, int status, std::vector<H2Header> headers, std::string body);
      void complete_stream(int32_t stream_id);

      static int on_begin_headers(nghttp2_session* session, const nghttp2_frame* frame, void* user_data);
      static int on_header(nghttp2_session* session,
                           const nghttp2_frame* frame,
                           const std::uint8_t* name,
                           std::size_t name_len,
                           const std::uint8_t* value,
                           std::size_t value_len,
                           std::uint8_t flags,
                           void* user_data);
      static int on_data_chunk(nghttp2_session* session,
                               std::uint8_t flags,
                               int32_t stream_id,
                               const std::uint8_t* data,
                               std::size_t len,
                               void* user_data);
      static int on_frame_recv(nghttp2_session* session, const nghttp2_frame* frame, void* user_data);
      static int on_stream_close(nghttp2_session* session,
                                 int32_t stream_id,
                                 std::uint32_t error_code,
                                 void* user_data);
      static nghttp2_ssize on_read_body(nghttp2_session* session,
                                        int32_t stream_id,
                                        std::uint8_t* buf,
                                        std::size_t length,
                                        std::uint32_t* data_flags,
                                        nghttp2_data_source* source,
                                        void* user_data);

      IH2Application* application_ = nullptr;
      nghttp2_session* session_ = nullptr;
      H2RequestAssembler requests_;
      H2OutboundBodies bodies_;
      std::exception_ptr app_error_;
};
