#pragma once

/**
 * @file
 * @brief Server-side HTTP/2 protocol engine on top of nghttp2.
 * @ingroup h2
 */

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
 * @brief One HTTP/2 connection, server role only, on top of nghttp2.
 *
 * It owns the nghttp2 session and nothing else: bytes in, bytes out, and one
 * completed request at a time handed to the application. No I/O happens here
 * (see H2ConnectionRunner), there is no HTTP/1.1 parser and no upgrade from
 * HTTP/1.1. The server SETTINGS frame is queued in the constructor with
 * nghttp2's defaults, so extended CONNECT (RFC 8441) is not offered.
 *
 * Requests are assembled by H2RequestAssembler and passed to
 * IH2Application::on_request() when END_STREAM arrives. If the application
 * does not submit a response, the session answers 500 "missing response". If
 * it throws, the exception is stored and rethrown from consume().
 *
 * Not thread-safe. Use it from the coroutine that runs the connection.
 */
class H2Session final : public IH2Engine {
   public:
      /**
       * @param application Receives the requests. Not owned; must outlive the session.
       * @throws std::invalid_argument If @p application is null.
       * @throws std::runtime_error    If nghttp2 cannot create the session or queue SETTINGS.
       */
      explicit H2Session(IH2Application* application);
      ~H2Session() override;

      H2Session(const H2Session&) = delete;
      H2Session& operator=(const H2Session&) = delete;

      /**
       * @copydoc IH2Engine::consume
       *
       * nghttp2 may call the application from here, once per completed
       * request. The application's exception is rethrown as is; nghttp2
       * errors are thrown as std::runtime_error.
       */
      void consume(const std::uint8_t* data, std::size_t len) override;

      /**
       * @copydoc IH2Engine::next_outbound
       *
       * Returns the bytes from nghttp2_session_mem_send2().
       */
      std::vector<std::uint8_t> next_outbound() override;

      /**
       * @copydoc IH2Engine::wants_read
       *
       * Returns nghttp2_session_want_read().
       */
      bool wants_read() const override;

      /**
       * @copydoc IH2Engine::wants_write
       *
       * Returns nghttp2_session_want_write().
       */
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
