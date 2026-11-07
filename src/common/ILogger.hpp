#pragma once

/**
 * @file
 * @brief Logging interface used by the server and the OCSP cache.
 * @ingroup common
 */

#include <string>

/**
 * @brief Destination of informational and error messages.
 *
 * Implementations may be called from several threads (the HTTP io_context
 * and the WebSocket thread) and must serialise their output themselves.
 * Tests use a mock to check what was reported.
 */
class ILogger {
   public:
      virtual ~ILogger() = default;

      /**
       * @brief Logs a normal operational message, such as the listening address.
       * @param message Text to log.
       */
      virtual void info(const std::string& message) = 0;

      /**
       * @brief Logs an error.
       * @param context Where the error happened, for example "HTTP session".
       * @param message Error description.
       */
      virtual void error(const std::string& context, const std::string& message) = 0;
};
