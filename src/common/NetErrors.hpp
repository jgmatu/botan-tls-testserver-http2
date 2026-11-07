#pragma once

/**
 * @file
 * @brief Classification and logging of network errors from coroutines.
 * @ingroup common
 */

#include "common/ILogger.hpp"

#include <boost/system/error_code.hpp>

#include <exception>
#include <functional>
#include <memory>
#include <string>

/**
 * @brief Tells whether an error only means that the peer went away.
 *
 * Session code catches these and ends the connection quietly instead of
 * logging them.
 *
 * @param ec Error from an Asio or TLS stream operation.
 * @return True for eof, connection_reset, operation_aborted, broken_pipe and not_connected.
 */
bool is_benign_disconnect(const boost::system::error_code& ec);

/**
 * @brief Reports an exception through the logger as "<context>: <what>".
 *
 * Does nothing when @p error is null. Exceptions that do not derive from
 * std::exception are reported as "unknown exception".
 *
 * @param logger  Destination of the message.
 * @param context Prefix that says where the exception escaped from.
 * @param error   Exception to report, may be null.
 */
void log_exception(ILogger& logger, const std::string& context, std::exception_ptr error);

/**
 * @brief Builds a co_spawn completion handler that logs escaped exceptions.
 * @param logger  Logger kept alive by the handler.
 * @param context Prefix passed to log_exception().
 * @return Handler suitable as the third argument of boost::asio::co_spawn.
 */
std::function<void(std::exception_ptr)> make_completion_handler(std::shared_ptr<ILogger> logger, std::string context);
