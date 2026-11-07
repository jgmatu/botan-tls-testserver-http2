#pragma once

#include "server/ISessionHandler.hpp"

/// WebSocket (RFC 6455) upgrade and the seconds counter.
class WebSocketProtocolHandler final : public IProtocolHandler {
   public:
      boost::asio::awaitable<void> serve(TlsStream& stream,
                                         std::shared_ptr<const IConnectionDetailsProvider> details) override;
};
