#pragma once

/**
 * @file
 * @brief Interface used by the application to answer one HTTP/2 stream.
 * @ingroup h2
 */

#include "http/h2/H2Message.hpp"

#include <string>
#include <vector>

/**
 * @brief Queues one response for the stream that just completed.
 *
 * H2Session creates one responder per request and passes it to
 * IH2Application::on_request(). It is valid only for the duration of that
 * call: the response is queued in nghttp2 immediately and sent by
 * H2ConnectionRunner afterwards. If the application returns without
 * submitting, the session answers 500 "missing response".
 */
class IH2Responder {
   public:
      virtual ~IH2Responder() = default;

      /**
       * @brief Queues the response.
       *
       * A content-length header is added for the body unless @p headers
       * already contains one. An empty @p body sends HEADERS with END_STREAM
       * and no DATA frame.
       *
       * @param status  HTTP status code.
       * @param headers Response headers without pseudo-headers. Names are lowercased.
       * @param body    Response body, may be empty.
       * @throws std::runtime_error If nghttp2 rejects the response.
       */
      virtual void submit(int status, std::vector<H2Header> headers, std::string body) = 0;

      /**
       * @brief Whether submit() has been called.
       * @return True once a response has been queued.
       */
      virtual bool submitted() const = 0;
};
