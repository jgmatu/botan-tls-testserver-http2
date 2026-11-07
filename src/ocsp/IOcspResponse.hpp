#pragma once

/**
 * @file
 * @brief Interface over an OCSP response.
 * @ingroup ocsp
 */

#include <botan/pkix_enums.h>
#include <botan/x509cert.h>

#include <chrono>
#include <cstdint>
#include <vector>

/**
 * @brief What the cache needs to know about an OCSP response; wraps Botan::OCSP::Response.
 *
 * An interface so OcspCache and estimate_cache_duration() can be tested
 * with responses that are valid for chosen time ranges.
 */
class IOcspResponse {
   public:
      virtual ~IOcspResponse() = default;

      /**
       * @brief Whether the responder answered with the "successful" response status.
       * @return True for OCSPResponseStatus successful(0).
       */
      virtual bool successful() const = 0;

      /**
       * @brief Status of @p subject according to this response at a given time.
       * @param issuer   Certificate that issued @p subject.
       * @param subject  Certificate whose status is checked.
       * @param ref_time Time at which the response is evaluated.
       * @return OCSP_RESPONSE_GOOD when the certificate is good at @p ref_time; OCSP_NOT_YET_VALID,
       *         OCSP_HAS_EXPIRED, revoked or other codes otherwise.
       */
      virtual Botan::Certificate_Status_Code status_for(const Botan::X509_Certificate& issuer,
                                                        const Botan::X509_Certificate& subject,
                                                        std::chrono::system_clock::time_point ref_time) const = 0;

      /**
       * @brief The DER encoding of the response, as stapled in the handshake.
       * @return The encoded response.
       */
      virtual std::vector<std::uint8_t> raw_bits() const = 0;
};
