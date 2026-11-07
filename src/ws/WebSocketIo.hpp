#pragma once

/**
 * @file
 * @brief Coroutine write helpers for the WebSocket code.
 * @ingroup ws
 */

#include "ws/WebSocketFrame.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/write.hpp>

#include <string_view>

/**
 * @brief Writes raw bytes, such as the handshake response.
 * @tparam Stream Asio write stream (socket or Botan::TLS::Stream).
 * @param stream Stream to write to.
 * @param bytes  Bytes to write. Must stay valid until the coroutine resumes.
 * @return Awaitable that completes when all bytes are written.
 * @throws boost::system::system_error On a write error.
 */
template <typename Stream>
boost::asio::awaitable<void> write_bytes(Stream& stream, std::string_view bytes) {
   co_await boost::asio::async_write(stream, boost::asio::buffer(bytes), boost::asio::use_awaitable);
}

/**
 * @brief Encodes and writes one server frame (see encode_frame()).
 * @tparam Stream Asio write stream (socket or Botan::TLS::Stream).
 * @param stream  Stream to write to.
 * @param opcode  Frame opcode (see ws_opcode).
 * @param payload Frame payload.
 * @return Awaitable that completes when the frame is written.
 * @throws boost::system::system_error On a write error.
 */
template <typename Stream>
boost::asio::awaitable<void> write_frame(Stream& stream, std::uint8_t opcode, std::string_view payload) {
   const auto frame = encode_frame(opcode, payload);
   co_await boost::asio::async_write(stream, boost::asio::buffer(frame), boost::asio::use_awaitable);
}
