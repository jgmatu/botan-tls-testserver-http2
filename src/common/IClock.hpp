#pragma once

/**
 * @file
 * @brief Wall clock abstraction.
 * @ingroup common
 */

#include <chrono>

/**
 * @brief Wall clock seam so time dependent logic can be tested without sleeping.
 *
 * The OCSP cache uses it to decide whether a stored response is still fresh.
 * Tests inject a fake clock.
 */
class IClock {
   public:
      virtual ~IClock() = default;

      /**
       * @brief Current time.
       * @return The current wall clock time.
       */
      virtual std::chrono::system_clock::time_point now() const = 0;
};

/// @brief IClock backed by std::chrono::system_clock.
class SystemClock final : public IClock {
   public:
      /// @copydoc IClock::now
      std::chrono::system_clock::time_point now() const override { return std::chrono::system_clock::now(); }
};
