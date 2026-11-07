#pragma once

#include "ws/SecondsCounterPump.hpp"
#include "ws/WebSocketFrame.hpp"
#include "ws/WebSocketHandshake.hpp"
#include "ws/WebSocketIo.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <array>
#include <string>

struct HandshakeResult {
      bool ok = false;
      std::string leftover;  ///< bytes the client sent after the upgrade request
};

/// Reads the HTTP/1.1 upgrade request and answers it with 101, or with 400 when it is not acceptable.
template <typename Stream>
boost::asio::awaitable<HandshakeResult> perform_websocket_handshake(Stream& stream) {
   std::string raw;
   std::array<char, 1024> buf{};
   while(raw.find("\r\n\r\n") == std::string::npos) {
      if(raw.size() > kMaxHandshakeSize) {
         co_await write_bytes(stream, kBadRequestResponse);
         co_return HandshakeResult{};
      }
      const std::size_t n = co_await stream.async_read_some(boost::asio::buffer(buf), boost::asio::use_awaitable);
      if(n == 0) {
         co_return HandshakeResult{};
      }
      raw.append(buf.data(), n);
   }

   const auto split = raw.find("\r\n\r\n");
   const HandshakeRequest request = parse_handshake_request(std::string_view(raw).substr(0, split));
   if(!is_valid_upgrade_request(request)) {
      co_await write_bytes(stream, kBadRequestResponse);
      co_return HandshakeResult{};
   }

   co_await write_bytes(stream, switching_protocols_response(request.key));
   co_return HandshakeResult{true, raw.substr(split + 4)};
}

/// A complete WebSocket connection on an already established (TLS) stream.
template <typename Stream>
boost::asio::awaitable<void> serve_websocket(Stream& stream) {
   const HandshakeResult handshake = co_await perform_websocket_handshake(stream);
   if(!handshake.ok) {
      co_return;
   }
   FrameParser parser;
   if(!handshake.leftover.empty()) {
      parser.append(handshake.leftover);
   }
   co_await pump_seconds_counter(stream, parser);
}
