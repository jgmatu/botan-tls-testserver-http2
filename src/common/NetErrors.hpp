#pragma once

#include "common/ILogger.hpp"

#include <boost/system/error_code.hpp>

#include <exception>
#include <functional>
#include <memory>
#include <string>

/// True for errors that just mean "the peer went away" and are not worth reporting.
bool is_benign_disconnect(const boost::system::error_code& ec);

/// Reports the exception (if any) through the logger as "<context>: <what>".
void log_exception(ILogger& logger, const std::string& context, std::exception_ptr error);

/// Completion handler for co_spawn that logs escaped exceptions.
std::function<void(std::exception_ptr)> make_completion_handler(std::shared_ptr<ILogger> logger, std::string context);
