#pragma once

/**
 * @file
 * @brief Demo WebSocket service: pushes a seconds counter to the client.
 * @ingroup ws
 */

#include "common/StreamCancel.hpp"
#include "ws/WebSocketFrame.hpp"
#include "ws/WebSocketIo.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/system/error_code.hpp>

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

/**
 * @brief Sends a seconds counter after the handshake and answers control frames.
 *
 * Sends the text frame "0" immediately, then "1", "2", ... once per second.
 * Answers ping with a pong carrying the same payload. Answers close with a
 * close frame that echoes the client's status code, or 1000 if it sent none,
 * and then returns. Text and binary frames from the client are ignored.
 *
 * Botan::TLS::Stream does not allow a write while a read is pending, so the
 * loop only ever has one operation in flight. The one-second timer marks the
 * tick as due and cancels the pending read (cancel_pending_io()). The
 * coroutine then writes the counter and reads again.
 *
 * @tparam Stream Asio stream (socket or Botan::TLS::Stream).
 * @param stream Stream on which the 101 response has been sent.
 * @param parser Parser already holding any bytes received after the upgrade request.
 * @return Awaitable that completes after a close frame, end of stream or a read error.
 * @throws boost::system::system_error On a write error.
 * @throws std::runtime_error          When the client sends an invalid frame (see FrameParser::next()).
 */
template <typename Stream>
boost::asio::awaitable<void> pump_seconds_counter(Stream& stream, FrameParser& parser) {
   namespace net = boost::asio;
   auto executor = co_await net::this_coro::executor;

   struct Tick {
         std::atomic<bool> alive{true};
         std::atomic<bool> due{false};
         std::atomic<bool> writing{false};
         std::atomic<std::uint64_t> epoch{0};
   };

   auto tick = std::make_shared<Tick>();
   net::steady_timer timer(executor);

   struct DisarmTick {
         Tick* tick = nullptr;
         net::steady_timer* timer = nullptr;

         ~DisarmTick() {
            if(tick == nullptr) {
               return;
            }
            tick->alive.store(false);
            tick->epoch.fetch_add(1);
            timer->cancel();
         }
   } disarm{tick.get(), &timer};

   std::uint64_t seconds = 0;

   auto arm = [&](std::chrono::steady_clock::time_point when) {
      const auto epoch = tick->epoch.fetch_add(1) + 1;
      timer.expires_at(when);
      timer.async_wait([&stream, tick, epoch](boost::system::error_code ec) {
         if(ec || !tick->alive.load() || tick->writing.load() || tick->epoch.load() != epoch) {
            return;
         }
         tick->due.store(true);
         cancel_pending_io(stream);
      });
   };

   auto begin_write = [&] {
      tick->writing.store(true);
      tick->epoch.fetch_add(1);
      timer.cancel();
   };
   auto end_write = [&] { tick->writing.store(false); };

   co_await write_frame(stream, ws_opcode::text, "0");
   auto next_tick = std::chrono::steady_clock::now() + std::chrono::seconds(1);

   while(true) {
      while(const std::optional<WsFrame> frame = parser.next()) {
         if(frame->opcode == ws_opcode::close) {
            begin_write();
            std::string payload;
            if(frame->payload.size() >= 2) {
               payload = frame->payload.substr(0, 2);
            } else {
               payload.push_back(static_cast<char>(0x03));
               payload.push_back(static_cast<char>(0xe8));
            }
            co_await write_frame(stream, ws_opcode::close, payload);
            end_write();
            co_return;
         }
         if(frame->opcode == ws_opcode::ping) {
            begin_write();
            co_await write_frame(stream, ws_opcode::pong, frame->payload);
            end_write();
         }
      }

      arm(next_tick);
      std::array<char, 4096> buf{};
      boost::system::error_code ec;
      const std::size_t n =
         co_await stream.async_read_some(net::buffer(buf), net::redirect_error(net::use_awaitable, ec));
      begin_write();
      if(n > 0) {
         parser.append(std::string_view(buf.data(), n));
      }
      if(tick->due.exchange(false)) {
         ++seconds;
         co_await write_frame(stream, ws_opcode::text, std::to_string(seconds));
         next_tick += std::chrono::seconds(1);
      }
      end_write();

      if(ec == net::error::operation_aborted) {
         continue;
      }
      if(ec || n == 0) {
         break;
      }
   }
}
