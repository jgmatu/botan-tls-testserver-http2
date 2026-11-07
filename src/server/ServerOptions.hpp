#pragma once

#include <cstdint>
#include <string>

struct ServerOptions {
      std::uint16_t port = 0;
      std::uint16_t ws_port = 50444;
      std::string policy = "default";
      std::string cert;
      std::string key;
      std::uint64_t ocsp_request_timeout_seconds = 10;
      std::uint64_t ocsp_cache_minutes = 6 * 60;
      std::string document_root = "webroot";
};
