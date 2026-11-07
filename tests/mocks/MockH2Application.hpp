#pragma once

#include "http/h2/IH2Application.hpp"

#include <gmock/gmock.h>

class MockH2Application : public IH2Application {
   public:
      MOCK_METHOD(void, on_request, (const H2Request& request, IH2Responder& respond), (override));
};
