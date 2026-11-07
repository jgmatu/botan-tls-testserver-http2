#pragma once

#include "common/IClock.hpp"

#include <gmock/gmock.h>

class MockClock : public IClock {
   public:
      MOCK_METHOD(std::chrono::system_clock::time_point, now, (), (const, override));
};

/// Clock that only moves when told to.
class FakeClock final : public IClock {
   public:
      std::chrono::system_clock::time_point now() const override { return now_; }

      void advance(std::chrono::system_clock::duration by) { now_ += by; }

   private:
      std::chrono::system_clock::time_point now_ = std::chrono::system_clock::time_point{} + std::chrono::hours(1000);
};
