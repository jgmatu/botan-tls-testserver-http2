#pragma once

/**
 * @file
 * @brief Minimal URL splitting for the OCSP responder address.
 * @ingroup ocsp
 */

#include <optional>
#include <string>

/// @brief Parts of a responder URL.
struct Url {
      std::string protocol;             ///< "http" when the URL has no explicit port, otherwise empty.
      std::string host;                 ///< Host name or address, without the port.
      std::optional<std::string> port;  ///< Port given in the URL, if any.
      std::string target;               ///< Path and query, "/" when the URL has none.
};

/**
 * @brief Splits "scheme://host[:port][/target]".
 *
 * The scheme itself is not stored. The protocol is only filled ("http") when
 * the URL carries no explicit port. IPv6 literals in brackets are not supported.
 *
 * @param url Responder URL from the certificate.
 * @return The parts, or std::nullopt when there is no "://".
 */
std::optional<Url> parse_url(const std::string& url);
