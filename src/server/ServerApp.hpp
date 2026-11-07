#pragma once

/**
 * @file
 * @brief Composition root of the server.
 * @ingroup server
 */

#include "common/ILogger.hpp"
#include "server/ServerOptions.hpp"

#include <memory>

/**
 * @brief Wires everything together and runs the HTTP/2 and WebSocket listeners until they stop.
 *
 * Builds the TLS context, the OCSP cache, the static file handler and the two
 * protocol handlers. It then starts two listeners on 0.0.0.0:
 * - the HTTPS port (ServerOptions::port), on an io_context run by one thread
 *   per hardware core. ALPN h2 goes to HTTP/2, http/1.1 to the WebSocket;
 * - the WebSocket port (ServerOptions::ws_port), on its own io_context and
 *   thread.
 */
class ServerApp {
   public:
      /**
       * @param options Validated options from parse_command_line().
       * @param logger  Logger for listeners, sessions and the OCSP cache.
       */
      ServerApp(ServerOptions options, std::shared_ptr<ILogger> logger);

      /**
       * @brief Runs the server on the calling thread.
       *
       * Blocks until the HTTPS io_context stops. The WebSocket io_context is
       * stopped and its thread joined before returning.
       *
       * @return 0 on a normal stop, 1 when setup or the event loop threw (the error is logged).
       */
      int run();

   private:
      ServerOptions options_;
      std::shared_ptr<ILogger> logger_;
};
