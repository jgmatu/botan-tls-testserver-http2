#pragma once

#include "common/NetErrors.hpp"
#include "common/StreamCancel.hpp"
#include "http/h2/IH2Engine.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

/**
 * Pumps bytes between an Asio stream and an HTTP/2 engine until the engine
 * is done, the peer disconnects or the connection stays idle for too long.
 */
class H2ConnectionRunner {
   public:
      explicit H2ConnectionRunner(IH2Engine& engine, std::chrono::milliseconds idle_timeout = std::chrono::seconds(30)) :
            engine_(engine), idle_timeout_(idle_timeout) {}

      template <typename AsyncStream>
      boost::asio::awaitable<void> run(AsyncStream& stream);

   private:
      template <typename AsyncStream>
      boost::asio::awaitable<bool> write_pending(AsyncStream& stream);

      IH2Engine& engine_;
      std::chrono::milliseconds idle_timeout_;
};

template <typename AsyncStream>
boost::asio::awaitable<bool> H2ConnectionRunner::write_pending(AsyncStream& stream) {
   for(;;) {
      const std::vector<std::uint8_t> bytes = engine_.next_outbound();
      if(bytes.empty()) {
         co_return true;
      }

      boost::system::error_code ec;
      co_await boost::asio::async_write(
         stream, boost::asio::buffer(bytes), boost::asio::redirect_error(boost::asio::use_awaitable, ec));
      if(is_benign_disconnect(ec)) {
         co_return false;
      }
      if(ec) {
         throw boost::system::system_error(ec);
      }
   }
}

template <typename AsyncStream>
boost::asio::awaitable<void> H2ConnectionRunner::run(AsyncStream& stream) {
   auto executor = co_await boost::asio::this_coro::executor;

   struct Idle {
         std::atomic<bool> alive{true};
         std::atomic<std::uint64_t> epoch{0};
   };

   auto idle = std::make_shared<Idle>();
   boost::asio::steady_timer idle_timer(executor);
   const auto idle_timeout = idle_timeout_;

   auto cancel_stream = [&stream] { cancel_pending_io(stream); };

   auto arm_idle = [&] {
      if(idle_timeout.count() <= 0) {
         return;
      }
      const auto epoch = idle->epoch.fetch_add(1) + 1;
      idle_timer.expires_after(idle_timeout);
      idle_timer.async_wait([idle, epoch, cancel_stream](boost::system::error_code ec) {
         if(ec || !idle->alive.load() || idle->epoch.load() != epoch) {
            return;
         }
         cancel_stream();
      });
   };

   auto disarm_idle = [&] {
      idle->alive.store(false);
      idle_timer.cancel();
   };

   try {
      while(engine_.wants_read() || engine_.wants_write()) {
         arm_idle();
         if(!co_await write_pending(stream)) {
            disarm_idle();
            co_return;
         }
         if(!engine_.wants_read()) {
            continue;
         }

         std::array<std::uint8_t, 16384> buffer{};
         boost::system::error_code ec;
         const std::size_t n = co_await stream.async_read_some(
            boost::asio::buffer(buffer), boost::asio::redirect_error(boost::asio::use_awaitable, ec));
         if(n > 0) {
            engine_.consume(buffer.data(), n);
         }
         if(is_benign_disconnect(ec) || n == 0) {
            disarm_idle();
            co_return;
         }
         if(ec) {
            disarm_idle();
            throw boost::system::system_error(ec);
         }
      }
   } catch(...) {
      disarm_idle();
      throw;
   }
   disarm_idle();
}
