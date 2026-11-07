#pragma once

#include "tls/IConnectionDetailsProvider.hpp"

#include <gmock/gmock.h>

class MockConnectionDetailsProvider : public IConnectionDetailsProvider {
   public:
      MOCK_METHOD(std::string, connection_details_json, (), (const, override));
};
