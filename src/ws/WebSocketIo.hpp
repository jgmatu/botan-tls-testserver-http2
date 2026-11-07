#pragma once

#include "ws/WebSocketFrame.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/write.hpp>

#include <string_view>

template <typename Stream>
boost::asio::awaitable<void> write_bytes(Stream& stream, std::string_view bytes) {
   co_await boost::asio::async_write(stream, boost::asio::buffer(bytes), boost::asio::use_awaitable);
}

template <typename Stream>
boost::asio::awaitable<void> write_frame(Stream& stream, std::uint8_t opcode, std::string_view payload) {
   const auto frame = encode_frame(opcode, payload);
   co_await boost::asio::async_write(stream, boost::asio::buffer(frame), boost::asio::use_awaitable);
}
