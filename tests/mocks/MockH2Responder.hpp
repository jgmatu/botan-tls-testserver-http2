#pragma once

#include "http/h2/IH2Responder.hpp"

#include <gmock/gmock.h>

#include <algorithm>

class MockH2Responder : public IH2Responder {
   public:
      MOCK_METHOD(void, submit, (int status, std::vector<H2Header> headers, std::string body), (override));
      MOCK_METHOD(bool, submitted, (), (const, override));
};

/// Matches a response header (name and value) in a std::vector<H2Header>.
MATCHER_P2(HasHeader, name, value, "") {
   return std::ranges::any_of(arg, [&](const H2Header& h) { return h.name == name && h.value == value; });
}

inline H2Request make_request(std::string method, std::string path, std::string body = {}) {
   H2Request request;
   request.stream_id = 1;
   request.method = std::move(method);
   request.path = std::move(path);
   request.body = std::move(body);
   return request;
}
