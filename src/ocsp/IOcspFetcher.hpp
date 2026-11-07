#pragma once

#include "ocsp/IOcspResponse.hpp"

#include <botan/x509cert.h>

#include <memory>

/// Asks the certificate's OCSP responder for a fresh response.
class IOcspFetcher {
   public:
      virtual ~IOcspFetcher() = default;

      /// Null when there is no responder or the request failed.
      virtual std::unique_ptr<IOcspResponse> fetch(const Botan::X509_Certificate& issuer,
                                                   const Botan::X509_Certificate& subject) = 0;
};
