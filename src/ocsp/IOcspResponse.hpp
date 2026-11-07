#pragma once

#include <botan/pkix_enums.h>
#include <botan/x509cert.h>

#include <chrono>
#include <cstdint>
#include <vector>

/// What the cache needs to know about an OCSP response; wraps Botan::OCSP::Response.
class IOcspResponse {
   public:
      virtual ~IOcspResponse() = default;

      /// The responder answered with the "successful" response status.
      virtual bool successful() const = 0;

      virtual Botan::Certificate_Status_Code status_for(const Botan::X509_Certificate& issuer,
                                                        const Botan::X509_Certificate& subject,
                                                        std::chrono::system_clock::time_point ref_time) const = 0;

      virtual std::vector<std::uint8_t> raw_bits() const = 0;
};
