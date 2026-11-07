#pragma once

/**
 * @file
 * @brief Interface for fetching OCSP responses from the CA.
 * @ingroup ocsp
 */

#include "ocsp/IOcspResponse.hpp"

#include <botan/x509cert.h>

#include <memory>

/**
 * @brief Asks the certificate's OCSP responder for a fresh response.
 *
 * OcspCache calls it on a cache miss. BeastOcspFetcher is the production
 * implementation; tests use a mock.
 */
class IOcspFetcher {
   public:
      virtual ~IOcspFetcher() = default;

      /**
       * @brief Fetches a response for @p subject.
       * @param issuer  Certificate that issued @p subject.
       * @param subject Certificate whose status is requested.
       * @return The response, or null when there is no responder or it did not answer with HTTP 200.
       * @throws std::exception Implementations may throw on network errors.
       */
      virtual std::unique_ptr<IOcspResponse> fetch(const Botan::X509_Certificate& issuer,
                                                   const Botan::X509_Certificate& subject) = 0;
};
