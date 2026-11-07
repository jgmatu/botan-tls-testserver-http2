#pragma once

/**
 * @file
 * @brief Console implementation of ILogger.
 * @ingroup common
 */

#include "common/ILogger.hpp"

#include <mutex>

/**
 * @brief Logger that writes info() to stdout and error() to stderr.
 *
 * Errors are prefixed with the local time from std::ctime(). A mutex
 * serialises both streams, so it is safe to share one instance between threads.
 */
class StderrLogger final : public ILogger {
   public:
      /**
       * @copydoc ILogger::info
       *
       * Writes @p message and a newline to stdout.
       */
      void info(const std::string& message) override;

      /**
       * @copydoc ILogger::error
       *
       * Writes the timestamp, @p context and @p message to stderr and flushes.
       */
      void error(const std::string& context, const std::string& message) override;

   private:
      std::mutex mutex_;
};
