#pragma once

#include <cstdint>
#include <string>

struct ConnectionDetails {
      std::string kex_algo;
      bool is_quantum_safe = false;
      std::uint16_t wire_code = 0;
      std::uint16_t websocket_port = 0;
};

/// The JSON document served by GET /api/connection_details.
std::string to_json(const ConnectionDetails& details);
