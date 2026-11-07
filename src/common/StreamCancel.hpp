#pragma once

/**
 * @file
 * @brief Cancellation of pending I/O on a plain or TLS-wrapped socket.
 * @ingroup common
 */

#include <boost/system/error_code.hpp>

/**
 * @brief Cancels pending async operations on a socket or on a TLS stream wrapping one.
 *
 * For a stream with next_layer() (such as Botan::TLS::Stream) the underlying
 * socket is cancelled; otherwise the stream itself. Errors from cancel() are
 * ignored. A stream that offers neither is left untouched.
 *
 * Pending operations complete with boost::asio::error::operation_aborted,
 * which the timers in this project use to wake up a coroutine blocked in a read.
 *
 * @tparam AsyncStream Socket type or a stream whose next_layer() is a socket.
 * @param stream Stream to cancel.
 */
template <typename AsyncStream>
void cancel_pending_io(AsyncStream& stream) {
   boost::system::error_code ignore;
   if constexpr(requires(AsyncStream& s, boost::system::error_code ec) { s.next_layer().cancel(ec); }) {
      stream.next_layer().cancel(ignore);
   } else if constexpr(requires(AsyncStream& s, boost::system::error_code ec) { s.cancel(ec); }) {
      stream.cancel(ignore);
   }
}
