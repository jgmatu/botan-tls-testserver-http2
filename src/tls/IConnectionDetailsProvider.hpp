#pragma once

/**
 * @file
 * @brief Interface that exposes the TLS details of one connection.
 * @ingroup tls
 */

#include <string>

/**
 * @brief Describes the TLS connection of the current client (key exchange group, ...).
 *
 * ServerStreamCallbacks implements it, because the callbacks are where Botan
 * reports the negotiated group. ApiHandler reads it to answer
 * GET /api/connection_details. The provider must only be read after the
 * handshake has completed.
 */
class IConnectionDetailsProvider {
   public:
      virtual ~IConnectionDetailsProvider() = default;

      /**
       * @brief The connection details as JSON.
       * @return A JSON object as produced by to_json().
       */
      virtual std::string connection_details_json() const = 0;
};
