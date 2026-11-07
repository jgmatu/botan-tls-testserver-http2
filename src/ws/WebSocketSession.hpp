#pragma once

/**
 * @file
 * @brief A WebSocket connection from the upgrade request to the end.
 * @ingroup ws
 */

#include "ws/SecondsCounterPump.hpp"
#include "ws/WebSocketFrame.hpp"
#include "ws/WebSocketHandshake.hpp"
#include "ws/WebSocketIo.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <array>
#include <string>

/// @brief Outcome of perform_websocket_handshake().
struct HandshakeResult {
      bool ok = false;       ///< True when 101 was sent and the connection is now a WebSocket.
      std::string leftover;  ///< bytes the client sent after the upgrade request
};

/**
 * @brief Reads the HTTP/1.1 upgrade request and answers it.
 *
 * Answers 101 Switching Protocols when is_valid_upgrade_request() accepts the
 * request. Answers 400 when it does not, or when the head grows beyond
 * kMaxHandshakeSize without an empty line.
 *
 * @tparam Stream Asio stream (socket or Botan::TLS::Stream).
 * @param stream Established stream; the TLS handshake is already done.
 * @return ok is false after a 400 or when a read returned no bytes.
 * @throws boost::system::system_error On a read or write error, including eof before a full head.
 */
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

/**
 * @brief A complete WebSocket connection on an already established (TLS) stream.
 *
 * Performs the handshake, then hands any bytes that followed the request to
 * the frame parser and runs pump_seconds_counter(). Used by
 * WebSocketProtocolHandler on the HTTPS port (ALPN http/1.1) and on the
 * --ws-port listener.
 *
 * @tparam Stream Asio stream (socket or Botan::TLS::Stream).
 * @param stream Established stream. The caller shuts it down afterwards.
 * @return Awaitable that completes when the WebSocket connection is over.
 * @throws boost::system::system_error On a read or write error during the handshake, or a write error afterwards.
 * @throws std::runtime_error          On a WebSocket protocol violation by the client.
 */
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
