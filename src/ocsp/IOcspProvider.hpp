#pragma once

#include <botan/x509cert.h>

#include <cstdint>
#include <vector>

/// Source of the OCSP response stapled to the server certificate.
class IOcspProvider {
   public:
      virtual ~IOcspProvider() = default;

      /// Empty when no valid response is available.
      virtual std::vector<std::uint8_t> get_ocsp_response(const Botan::X509_Certificate& issuer,
                                                          const Botan::X509_Certificate& subject) = 0;
};
