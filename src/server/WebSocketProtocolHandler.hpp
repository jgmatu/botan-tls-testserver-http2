#pragma once

/**
 * @file
 * @brief IProtocolHandler for the WebSocket over HTTP/1.1 (ALPN http/1.1).
 * @ingroup server
 */

#include "server/ISessionHandler.hpp"

/**
 * @brief WebSocket (RFC 6455) upgrade and the seconds counter.
 *
 * Stateless; one instance serves both the HTTPS port and the --ws-port
 * listener. The connection details are not used.
 */
class WebSocketProtocolHandler final : public IProtocolHandler {
   public:
      /**
       * @copydoc IProtocolHandler::serve
       *
       * Runs serve_websocket() on the stream.
       */
      boost::asio::awaitable<void> serve(TlsStream& stream,
                                         std::shared_ptr<const IConnectionDetailsProvider> details) override;
};
