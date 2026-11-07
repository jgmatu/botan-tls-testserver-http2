#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct H2Header {
      std::string name;
      std::string value;
};

struct H2Request {
      int32_t stream_id = 0;
      std::string method;
      std::string path;
      std::vector<H2Header> headers;
      std::string body;
};
