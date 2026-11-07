#include "server/WebSocketProtocolHandler.hpp"

#include "ws/WebSocketSession.hpp"

boost::asio::awaitable<void> WebSocketProtocolHandler::serve(TlsStream& stream,
                                                             std::shared_ptr<const IConnectionDetailsProvider>) {
   co_await serve_websocket(stream);
}
