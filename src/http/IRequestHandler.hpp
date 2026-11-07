#pragma once

/**
 * @file
 * @brief Interface of one family of HTTP request handlers.
 * @ingroup http
 */

#include "http/h2/H2Message.hpp"
#include "http/h2/IH2Responder.hpp"

/**
 * @brief Answers one family of requests (the REST API, static files, ...).
 *
 * WebApplication picks the handler from the request path and forwards the
 * request unchanged. The handler is called on the connection's io_context,
 * once per completed HTTP/2 stream. It must answer synchronously through
 * @p respond and must not block on I/O.
 */
class IRequestHandler {
   public:
      virtual ~IRequestHandler() = default;

      /**
       * @brief Handles one request and submits exactly one response.
       * @param request Complete request: pseudo-headers, headers and body.
       * @param respond Responder for this stream. Only valid during the call.
       */
      virtual void handle(const H2Request& request, IH2Responder& respond) = 0;
};
