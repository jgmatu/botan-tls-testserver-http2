#pragma once

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
 * After the handshake: sends a seconds counter once per second, answers
 * ping with pong and close with close. Returns when the peer closes or goes away.
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
