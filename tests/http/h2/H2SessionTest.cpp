#include "http/h2/H2Session.hpp"

#include "mocks/MockH2Application.hpp"

#include <nghttp2/nghttp2.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Throw;

namespace {

nghttp2_nv make_nv(const std::string& name, const std::string& value) {
   nghttp2_nv nv{};
   nv.name = reinterpret_cast<std::uint8_t*>(const_cast<char*>(name.data()));
   nv.namelen = name.size();
   nv.value = reinterpret_cast<std::uint8_t*>(const_cast<char*>(value.data()));
   nv.valuelen = value.size();
   nv.flags = NGHTTP2_NV_FLAG_NONE;
   return nv;
}

/// A real nghttp2 client wired to an H2Session through memory.
class H2TestClient {
   public:
      struct Response {
            std::vector<std::pair<std::string, std::string>> headers;
            std::string body;
            bool closed = false;

            std::string header(const std::string& name) const {
               for(const auto& [n, v] : headers) {
                  if(n == name) {
                     return v;
                  }
               }
               return {};
            }
      };

      H2TestClient() {
         nghttp2_session_callbacks* callbacks = nullptr;
         nghttp2_session_callbacks_new(&callbacks);
         nghttp2_session_callbacks_set_on_header_callback(callbacks, &H2TestClient::on_header);
         nghttp2_session_callbacks_set_on_data_chunk_recv_callback(callbacks, &H2TestClient::on_data);
         nghttp2_session_callbacks_set_on_stream_close_callback(callbacks, &H2TestClient::on_close);
         nghttp2_session_client_new(&session_, callbacks, this);
         nghttp2_session_callbacks_del(callbacks);
         nghttp2_submit_settings(session_, NGHTTP2_FLAG_NONE, nullptr, 0);
      }

      ~H2TestClient() { nghttp2_session_del(session_); }

      H2TestClient(const H2TestClient&) = delete;
      H2TestClient& operator=(const H2TestClient&) = delete;

      int32_t request(const std::string& method, const std::string& path, std::string body = {}) {
         const std::vector<std::pair<std::string, std::string>> fields = {
            {":method", method},
            {":scheme", "https"},
            {":authority", "localhost"},
            {":path", path},
            {"user-agent", "h2-test"},
         };
         std::vector<nghttp2_nv> nva;
         for(const auto& [name, value] : fields) {
            nva.push_back(make_nv(name, value));
         }

         nghttp2_data_provider2 provider{};
         const nghttp2_data_provider2* data = nullptr;
         if(!body.empty()) {
            upload_ = std::make_unique<Upload>(Upload{std::move(body), 0});
            provider.source.ptr = upload_.get();
            provider.read_callback = &H2TestClient::read_upload;
            data = &provider;
         }
         return nghttp2_submit_request2(session_, nullptr, nva.data(), nva.size(), data, nullptr);
      }

      /// Moves bytes back and forth until neither side has anything left to say.
      void pump(H2Session& server) {
         for(int round = 0; round < 200; ++round) {
            bool moved = false;

            const std::uint8_t* data = nullptr;
            nghttp2_ssize n = 0;
            while((n = nghttp2_session_mem_send2(session_, &data)) > 0) {
               server.consume(data, static_cast<std::size_t>(n));
               moved = true;
            }

            for(auto bytes = server.next_outbound(); !bytes.empty(); bytes = server.next_outbound()) {
               nghttp2_session_mem_recv2(session_, bytes.data(), bytes.size());
               moved = true;
            }

            if(!moved) {
               return;
            }
         }
         FAIL() << "HTTP/2 exchange did not settle";
      }

      const Response& response(int32_t stream_id) { return responses_[stream_id]; }

   private:
      struct Upload {
            std::string data;
            std::size_t offset;
      };

      static int on_header(nghttp2_session*,
                           const nghttp2_frame* frame,
                           const std::uint8_t* name,
                           std::size_t name_len,
                           const std::uint8_t* value,
                           std::size_t value_len,
                           std::uint8_t,
                           void* user_data) {
         static_cast<H2TestClient*>(user_data)->responses_[frame->hd.stream_id].headers.emplace_back(
            std::string(reinterpret_cast<const char*>(name), name_len),
            std::string(reinterpret_cast<const char*>(value), value_len));
         return 0;
      }

      static int on_data(
         nghttp2_session*, std::uint8_t, int32_t stream_id, const std::uint8_t* data, std::size_t len, void* user_data) {
         static_cast<H2TestClient*>(user_data)->responses_[stream_id].body.append(
            reinterpret_cast<const char*>(data), len);
         return 0;
      }

      static int on_close(nghttp2_session*, int32_t stream_id, std::uint32_t, void* user_data) {
         static_cast<H2TestClient*>(user_data)->responses_[stream_id].closed = true;
         return 0;
      }

      static nghttp2_ssize read_upload(nghttp2_session*,
                                       int32_t,
                                       std::uint8_t* buf,
                                       std::size_t length,
                                       std::uint32_t* data_flags,
                                       nghttp2_data_source* source,
                                       void*) {
         auto* upload = static_cast<Upload*>(source->ptr);
         const std::size_t n = std::min(length, upload->data.size() - upload->offset);
         std::memcpy(buf, upload->data.data() + upload->offset, n);
         upload->offset += n;
         if(upload->offset == upload->data.size()) {
            *data_flags |= NGHTTP2_DATA_FLAG_EOF;
         }
         return static_cast<nghttp2_ssize>(n);
      }

      nghttp2_session* session_ = nullptr;
      std::unique_ptr<Upload> upload_;
      std::map<int32_t, Response> responses_;
};

}  // namespace

class H2SessionTest : public ::testing::Test {
   protected:
      ::testing::StrictMock<MockH2Application> app;
      H2Session server{&app};
      H2TestClient client;
};

TEST_F(H2SessionTest, RejectsMissingApplication) {
   EXPECT_THROW(H2Session(nullptr), std::invalid_argument);
}

TEST_F(H2SessionTest, StartsWithSettingsFrame) {
   EXPECT_TRUE(server.wants_read());
   EXPECT_TRUE(server.wants_write());

   const auto bytes = server.next_outbound();
   ASSERT_GE(bytes.size(), 9u);
   EXPECT_EQ(bytes[3], NGHTTP2_SETTINGS) << "the preface must begin with SETTINGS";
}

TEST_F(H2SessionTest, DeliversRequestAndSendsResponse) {
   H2Request seen;
   EXPECT_CALL(app, on_request(_, _)).WillOnce(Invoke([&](const H2Request& request, IH2Responder& respond) {
      seen = request;
      respond.submit(200, {{"Content-Type", "text/plain"}, {"X-Test", "yes"}}, "hello");
   }));

   const int32_t id = client.request("GET", "/index.html?x=1");
   client.pump(server);

   EXPECT_EQ(seen.stream_id, id);
   EXPECT_EQ(seen.method, "GET");
   EXPECT_EQ(seen.path, "/index.html?x=1");
   EXPECT_TRUE(seen.body.empty());
   EXPECT_THAT(seen.headers, ::testing::Contains(::testing::Field(&H2Header::name, "user-agent")));

   const auto& response = client.response(id);
   EXPECT_TRUE(response.closed);
   EXPECT_EQ(response.header(":status"), "200");
   EXPECT_EQ(response.header("content-type"), "text/plain");
   EXPECT_EQ(response.header("x-test"), "yes") << "header names are lower-cased";
   EXPECT_EQ(response.header("content-length"), "5");
   EXPECT_EQ(response.body, "hello");
}

TEST_F(H2SessionTest, DeliversRequestBody) {
   std::string received;
   EXPECT_CALL(app, on_request(_, _)).WillOnce(Invoke([&](const H2Request& request, IH2Responder& respond) {
      received = request.body;
      respond.submit(204, {}, {});
   }));

   const int32_t id = client.request("POST", "/upload", "payload-bytes");
   client.pump(server);

   EXPECT_EQ(received, "payload-bytes");
   EXPECT_EQ(client.response(id).header(":status"), "204");
}

TEST_F(H2SessionTest, ResponseWithoutBodyEndsTheStreamWithZeroLength) {
   EXPECT_CALL(app, on_request(_, _)).WillOnce(Invoke([](const H2Request&, IH2Responder& respond) {
      respond.submit(404, {}, {});
   }));

   const int32_t id = client.request("GET", "/missing");
   client.pump(server);

   const auto& response = client.response(id);
   EXPECT_TRUE(response.closed);
   EXPECT_EQ(response.header(":status"), "404");
   EXPECT_EQ(response.header("content-length"), "0");
   EXPECT_TRUE(response.body.empty());
}

TEST_F(H2SessionTest, LargeBodyIsDeliveredInFull) {
   const std::string big(200 * 1024, 'x');
   EXPECT_CALL(app, on_request(_, _)).WillOnce(Invoke([&](const H2Request&, IH2Responder& respond) {
      respond.submit(200, {}, big);
   }));

   const int32_t id = client.request("GET", "/big");
   client.pump(server);

   EXPECT_EQ(client.response(id).body.size(), big.size());
   EXPECT_EQ(client.response(id).body, big);
   EXPECT_TRUE(client.response(id).closed);
}

TEST_F(H2SessionTest, ApplicationThatDoesNotRespondYields500) {
   EXPECT_CALL(app, on_request(_, _));

   const int32_t id = client.request("GET", "/silent");
   client.pump(server);

   const auto& response = client.response(id);
   EXPECT_EQ(response.header(":status"), "500");
   EXPECT_EQ(response.body, "missing response");
}

TEST_F(H2SessionTest, ConcurrentStreamsAreAnsweredSeparately) {
   EXPECT_CALL(app, on_request(_, _)).Times(2).WillRepeatedly(Invoke([](const H2Request& request, IH2Responder& respond) {
      respond.submit(200, {}, "path=" + request.path);
   }));

   const int32_t first = client.request("GET", "/a");
   const int32_t second = client.request("GET", "/b");
   client.pump(server);

   EXPECT_EQ(client.response(first).body, "path=/a");
   EXPECT_EQ(client.response(second).body, "path=/b");
}

TEST_F(H2SessionTest, ApplicationExceptionIsRethrownFromConsume) {
   EXPECT_CALL(app, on_request(_, _)).WillOnce(Throw(std::runtime_error("app failed")));

   client.request("GET", "/boom");

   EXPECT_THROW(client.pump(server), std::runtime_error);
}

TEST_F(H2SessionTest, GarbageInputIsAProtocolError) {
   const std::string garbage(64, '\x7f');

   EXPECT_THROW(server.consume(reinterpret_cast<const std::uint8_t*>(garbage.data()), garbage.size()),
                std::runtime_error);
}
