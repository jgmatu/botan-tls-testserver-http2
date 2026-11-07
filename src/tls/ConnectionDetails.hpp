#pragma once

/**
 * @file
 * @brief Data and JSON encoding of GET /api/connection_details.
 * @ingroup tls
 */

#include <cstdint>
#include <string>

/// @brief What the test page shows about the client's TLS connection.
struct ConnectionDetails {
      std::string kex_algo;              ///< Botan name of the key exchange group, for example "x25519/ML-KEM-768".
      bool is_quantum_safe = false;      ///< True for pure or hybrid post-quantum groups.
      std::uint16_t wire_code = 0;       ///< TLS NamedGroup code, for example 0x11ec.
      std::uint16_t websocket_port = 0;  ///< Port the page should open the WebSocket on.
};

/**
 * @brief The JSON document served by GET /api/connection_details.
 *
 * Example: {"kex_algo": "x25519/ML-KEM-768","is_quantum_safe": true,"wire_code": "0x11ec","websocket_port": 50443}.
 * kex_algo is inserted without escaping; it always comes from Botan's group names.
 *
 * @param details Values to encode.
 * @return The JSON text. wire_code is a four-digit hex string.
 */
std::string to_json(const ConnectionDetails& details);
