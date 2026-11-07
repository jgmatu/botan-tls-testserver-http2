#pragma once

/**
 * @file
 * @brief RFC 6455 opening handshake: parsing, validation and the 101 response.
 * @ingroup ws
 */

#include <string>
#include <string_view>

/// @brief Fields of the client's HTTP/1.1 upgrade request that matter for RFC 6455.
struct HandshakeRequest {
      std::string method;      ///< Request method, for example "GET".
      std::string target;      ///< Request-target, for example "/ws".
      std::string version;     ///< HTTP version, for example "HTTP/1.1".
      std::string upgrade;     ///< Value of the Upgrade header.
      std::string connection;  ///< Value of the Connection header.
      std::string key;         ///< Value of Sec-WebSocket-Key.
      std::string ws_version;  ///< Value of Sec-WebSocket-Version.
};

/// Largest request head accepted before the handshake is answered with 400.
constexpr std::size_t kMaxHandshakeSize = 8192;

/// Response sent for a malformed or unacceptable upgrade request.
constexpr std::string_view kBadRequestResponse = "HTTP/1.1 400 Bad Request\r\nConnection: close\r\nContent-Length: 0\r\n\r\n";

/**
 * @brief Parses the request head.
 *
 * Header names are matched ignoring case and values are trimmed. Headers
 * other than the four in HandshakeRequest are ignored.
 *
 * @param head Everything before the empty line that ends the request head.
 * @return The parsed fields. Missing parts stay empty.
 */
HandshakeRequest parse_handshake_request(std::string_view head);

/**
 * @brief Looks for a token in a comma-separated header value.
 * @param value Header value, for example "keep-alive, Upgrade".
 * @param token Token to find.
 * @return True when one of the trimmed list items equals @p token, ignoring ASCII case.
 */
bool header_has_token(std::string_view value, std::string_view token);

/**
 * @brief Checks that the request is a WebSocket upgrade this server accepts.
 *
 * Accepted: GET /ws (query string allowed) over HTTP/1.0 or 1.1, Upgrade
 * containing "websocket", Connection containing "upgrade", a non-empty key and
 * Sec-WebSocket-Version 13.
 *
 * @param request Parsed request.
 * @return True when the upgrade can be answered with 101.
 */
bool is_valid_upgrade_request(const HandshakeRequest& request);

/**
 * @brief Sec-WebSocket-Accept value for a client key.
 * @param client_key Sec-WebSocket-Key sent by the client.
 * @return Base64 of SHA-1(key + RFC 6455 GUID).
 */
std::string accept_key(std::string_view client_key);

/**
 * @brief The complete "101 Switching Protocols" response.
 * @param client_key Sec-WebSocket-Key sent by the client.
 * @return The response head, terminated by an empty line.
 */
std::string switching_protocols_response(std::string_view client_key);
