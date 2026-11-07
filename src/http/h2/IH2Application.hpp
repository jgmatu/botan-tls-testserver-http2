#pragma once

/**
 * @file
 * @brief Interface between the HTTP/2 session and the application logic.
 * @ingroup h2
 */

#include "http/h2/H2Message.hpp"
#include "http/h2/IH2Responder.hpp"

/**
 * @brief Business entry point of an HTTP/2 connection.
 *
 * H2Session calls on_request() once per stream, after that stream's request
 * headers and body are complete (END_STREAM received). The application does
 * not see frames, flags or HPACK. WebApplication is the implementation used
 * by the server.
 *
 * The call happens inside H2Session::consume(), on the connection's
 * io_context, so it must not block. An exception thrown from on_request()
 * fails the nghttp2 callback, is rethrown from consume() and ends the
 * connection.
 */
class IH2Application {
   public:
      virtual ~IH2Application() = default;

      /**
       * @brief Handles one complete request.
       * @param request The request of the stream that just ended.
       * @param respond Responder for that stream. Only valid during the call.
       */
      virtual void on_request(const H2Request& request, IH2Responder& respond) = 0;
};
