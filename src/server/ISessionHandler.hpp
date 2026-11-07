#pragma once

/**
 * @file
 * @brief Interfaces for connection and protocol handlers, and the stream types they use.
 * @ingroup server
 */

#include "tls/IConnectionDetailsProvider.hpp"

#include <botan/asio_stream.h>

#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <memory>

/// TCP socket whose async operations default to boost::asio::use_awaitable.
using AwaitableTcp = boost::asio::use_awaitable_t<>::as_default_on_t<boost::asio::ip::tcp::socket>;

/// Botan TLS stream over an AwaitableTcp socket.
using TlsStream = Botan::TLS::Stream<AwaitableTcp>;

/**
 * @brief Owns one accepted TCP connection from start to finish.
 *
 * run_tcp_listener() spawns handle() on its own coroutine for every accepted
 * socket. TlsSessionHandler is the implementation. A single handler instance
 * serves all connections of a listener concurrently.
 */
class ISessionHandler {
   public:
      virtual ~ISessionHandler() = default;

      /**
       * @brief Runs the connection until it ends.
       * @param socket Accepted socket; the handler takes ownership.
       * @return Awaitable that completes when the connection is closed.
       * @throws std::exception Errors that are not a benign disconnect; the listener logs them.
       */
      virtual boost::asio::awaitable<void> handle(AwaitableTcp socket) = 0;
};

/**
 * @brief Speaks one application protocol (chosen by ALPN) on an established TLS stream.
 *
 * TlsSessionHandler calls serve() after the handshake and shuts the stream
 * down when it returns. Implementations do not close the stream themselves.
 * Http2ProtocolHandler and WebSocketProtocolHandler are the implementations.
 */
class IProtocolHandler {
   public:
      virtual ~IProtocolHandler() = default;

      /**
       * @brief Serves the protocol until the peer is done.
       * @param stream  Established TLS stream. Valid until the coroutine returns.
       * @param details TLS details of this connection, for handlers that report them.
       * @return Awaitable that completes when the protocol session is over.
       * @throws std::exception On I/O or protocol errors.
       */
      virtual boost::asio::awaitable<void> serve(TlsStream& stream,
                                                 std::shared_ptr<const IConnectionDetailsProvider> details) = 0;
};
