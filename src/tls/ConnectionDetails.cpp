#include "tls/ConnectionDetails.hpp"

#include <iomanip>
#include <sstream>

std::string to_json(const ConnectionDetails& details) {
   std::stringstream ss;
   ss << "{"
      << "\"kex_algo\": \"" << details.kex_algo << "\","
      << "\"is_quantum_safe\": " << (details.is_quantum_safe ? "true" : "false") << ","
      << "\"wire_code\": \"0x" << std::hex << std::setw(4) << std::setfill('0') << details.wire_code << "\","
      << std::dec << "\"websocket_port\": " << details.websocket_port << "}";
   return ss.str();
}
