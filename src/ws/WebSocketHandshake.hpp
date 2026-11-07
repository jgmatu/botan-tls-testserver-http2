#pragma once

#include <string>
#include <string_view>

/// Fields of the client's HTTP/1.1 upgrade request that matter for RFC 6455.
struct HandshakeRequest {
      std::string method;
      std::string target;
      std::string version;
      std::string upgrade;
      std::string connection;
      std::string key;
      std::string ws_version;
};

constexpr std::size_t kMaxHandshakeSize = 8192;

constexpr std::string_view kBadRequestResponse = "HTTP/1.1 400 Bad Request\r\nConnection: close\r\nContent-Length: 0\r\n\r\n";

/// Parses the request head (everything before the empty line). Missing parts stay empty.
HandshakeRequest parse_handshake_request(std::string_view head);

/// True when the header value, read as a comma separated list, contains the token (case-insensitive).
bool header_has_token(std::string_view value, std::string_view token);

/// GET /ws over HTTP/1.0 or 1.1 asking to upgrade to WebSocket version 13 with a key.
bool is_valid_upgrade_request(const HandshakeRequest& request);

/// Sec-WebSocket-Accept value for a client key.
std::string accept_key(std::string_view client_key);

/// The complete "101 Switching Protocols" response for a client key.
std::string switching_protocols_response(std::string_view client_key);
