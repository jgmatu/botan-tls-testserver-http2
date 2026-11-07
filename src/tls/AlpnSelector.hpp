#pragma once

/**
 * @file
 * @brief ALPN protocol selection.
 * @ingroup tls
 */

#include <string>
#include <vector>

/**
 * @brief Picks the application protocol for the TLS handshake.
 *
 * The server's preference wins over the client's order. On the HTTPS port the
 * preference is {"h2", "http/1.1"}: browsers get HTTP/2 for the page and
 * http/1.1 for the WebSocket, whose connection only offers http/1.1.
 *
 * @param client_protocols  Protocols offered by the client.
 * @param server_preference Protocols the server supports, most preferred first.
 * @return The first protocol of @p server_preference that the client also offers, or empty for none.
 */
std::string select_alpn_protocol(const std::vector<std::string>& client_protocols,
                                 const std::vector<std::string>& server_preference);
