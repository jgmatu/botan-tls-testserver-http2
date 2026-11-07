#include "ws/WebSocketHandshake.hpp"

#include "common/Strings.hpp"
#include "http/RequestTarget.hpp"

#include <botan/base64.h>
#include <botan/hash.h>

namespace {

constexpr std::string_view kWebsocketGuid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

}  // namespace

HandshakeRequest parse_handshake_request(std::string_view head_view) {
   const std::string head(head_view);
   HandshakeRequest request;

   const auto line_end = head.find("\r\n");
   const std::string request_line = line_end == std::string::npos ? head : head.substr(0, line_end);
   const std::size_t first = request_line.find(' ');
   const std::size_t second = first == std::string::npos ? std::string::npos : request_line.find(' ', first + 1);
   if(first != std::string::npos && second != std::string::npos) {
      request.method = request_line.substr(0, first);
      request.target = request_line.substr(first + 1, second - first - 1);
      request.version = request_line.substr(second + 1);
   }

   std::size_t cursor = line_end == std::string::npos ? head.size() : line_end + 2;
   while(cursor < head.size()) {
      auto next = head.find("\r\n", cursor);
      if(next == std::string::npos) {
         next = head.size();
      }
      const std::string line = head.substr(cursor, next - cursor);
      cursor = next + 2;
      const auto colon = line.find(':');
      if(colon != std::string::npos) {
         const std::string name = lower_copy(trim_copy(line.substr(0, colon)));
         const std::string value = trim_copy(line.substr(colon + 1));
         if(name == "upgrade") {
            request.upgrade = value;
         } else if(name == "connection") {
            request.connection = value;
         } else if(name == "sec-websocket-key") {
            request.key = value;
         } else if(name == "sec-websocket-version") {
            request.ws_version = value;
         }
      }
      if(next == head.size()) {
         break;
      }
   }

   return request;
}

bool header_has_token(std::string_view value, std::string_view token) {
   const std::string needle = lower_copy(token);
   std::size_t begin = 0;
   while(begin <= value.size()) {
      const auto comma = value.find(',', begin);
      const auto part = value.substr(begin, comma == std::string_view::npos ? std::string_view::npos : comma - begin);
      if(lower_copy(trim_copy(part)) == needle) {
         return true;
      }
      if(comma == std::string_view::npos) {
         break;
      }
      begin = comma + 1;
   }
   return false;
}

bool is_valid_upgrade_request(const HandshakeRequest& request) {
   return request.method == "GET" && request_path(request.target) == "/ws" &&
          (request.version == "HTTP/1.1" || request.version == "HTTP/1.0") &&
          header_has_token(request.upgrade, "websocket") && header_has_token(request.connection, "upgrade") &&
          !request.key.empty() && request.ws_version == "13";
}

std::string accept_key(std::string_view client_key) {
   auto hash = Botan::HashFunction::create_or_throw("SHA-1");
   hash->update(client_key);
   hash->update(kWebsocketGuid);
   const auto digest = hash->final();
   return Botan::base64_encode(digest.data(), digest.size());
}

std::string switching_protocols_response(std::string_view client_key) {
   return "HTTP/1.1 101 Switching Protocols\r\n"
          "Upgrade: websocket\r\n"
          "Connection: Upgrade\r\n"
          "Sec-WebSocket-Accept: " +
          accept_key(client_key) + "\r\n\r\n";
}
