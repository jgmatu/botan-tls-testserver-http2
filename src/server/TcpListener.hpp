#pragma once

/**
 * @file
 * @brief TCP accept loop.
 * @ingroup server
 */

#include "common/ILogger.hpp"
#include "server/ISessionHandler.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <memory>
#include <string>

/**
 * @brief Accepts connections forever and runs each one through the handler on its own coroutine.
 *
 * The acceptor uses SO_REUSEADDR. Each connection is co_spawned on the
 * listener's executor. Exceptions that escape a connection are logged with
 * @p session_context and do not stop the listener.
 *
 * @param endpoint        Address and port to bind.
 * @param handler         Handler for every accepted socket.
 * @param logger          Receives errors from connections.
 * @param session_context Prefix for those errors, for example "Session".
 * @return Awaitable that never completes normally; it runs until its io_context stops.
 * @throws boost::system::system_error When the port cannot be bound or accept fails.
 */
boost::asio::awaitable<void> run_tcp_listener(boost::asio::ip::tcp::endpoint endpoint,
                                              std::shared_ptr<ISessionHandler> handler,
                                              std::shared_ptr<ILogger> logger,
                                              std::string session_context);
