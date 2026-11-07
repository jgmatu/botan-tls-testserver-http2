#pragma once

#include <chrono>

/// Wall clock seam so time dependent logic can be tested without sleeping.
class IClock {
   public:
      virtual ~IClock() = default;

      virtual std::chrono::system_clock::time_point now() const = 0;
};

class SystemClock final : public IClock {
   public:
      std::chrono::system_clock::time_point now() const override { return std::chrono::system_clock::now(); }
};
