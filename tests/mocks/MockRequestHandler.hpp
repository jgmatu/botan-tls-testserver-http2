#pragma once

#include "http/IRequestHandler.hpp"

#include <gmock/gmock.h>

class MockRequestHandler : public IRequestHandler {
   public:
      MOCK_METHOD(void, handle, (const H2Request& request, IH2Responder& respond), (override));
};
