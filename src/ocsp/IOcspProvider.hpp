#pragma once

/**
 * @file
 * @brief Interface for the OCSP response stapled in the TLS handshake.
 * @ingroup ocsp
 */

#include <botan/x509cert.h>

#include <cstdint>
#include <vector>

/**
 * @brief Source of the OCSP response stapled to the server certificate.
 *
 * ServerStreamCallbacks::tls_provide_cert_status() calls it during the
 * handshake, from any io_context thread. Implementations must be thread-safe.
 * OcspCache is the implementation used by the server.
 */
class IOcspProvider {
   public:
      virtual ~IOcspProvider() = default;

      /**
       * @brief Returns a good OCSP response for @p subject.
       * @param issuer  Certificate that issued @p subject (chain[1]).
       * @param subject Server certificate (chain[0]).
       * @return The DER-encoded response, or empty when no valid response is available.
       */
      virtual std::vector<std::uint8_t> get_ocsp_response(const Botan::X509_Certificate& issuer,
                                                          const Botan::X509_Certificate& subject) = 0;
};
