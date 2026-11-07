#pragma once

#include "http/h2/IH2Engine.hpp"

#include <gmock/gmock.h>

class MockH2Engine : public IH2Engine {
   public:
      MOCK_METHOD(void, consume, (const std::uint8_t* data, std::size_t len), (override));
      MOCK_METHOD(std::vector<std::uint8_t>, next_outbound, (), (override));
      MOCK_METHOD(bool, wants_read, (), (const, override));
      MOCK_METHOD(bool, wants_write, (), (const, override));
};
