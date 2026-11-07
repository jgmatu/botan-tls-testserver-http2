#pragma once

/**
 * @file
 * @brief IOcspResponse adapter for Botan::OCSP::Response.
 * @ingroup ocsp
 */

#include "ocsp/IOcspResponse.hpp"

#include <botan/ocsp.h>

/// @brief IOcspResponse that forwards to a Botan::OCSP::Response.
class BotanOcspResponse final : public IOcspResponse {
   public:
      /// @param response Parsed response from the responder.
      explicit BotanOcspResponse(Botan::OCSP::Response response) : response_(std::move(response)) {}

      /**
       * @copydoc IOcspResponse::successful
       *
       * Compares Botan's status with Response_Status_Code::Successful.
       */
      bool successful() const override;

      /**
       * @copydoc IOcspResponse::status_for
       *
       * Forwards to Botan::OCSP::Response::status_for().
       */
      Botan::Certificate_Status_Code status_for(const Botan::X509_Certificate& issuer,
                                                const Botan::X509_Certificate& subject,
                                                std::chrono::system_clock::time_point ref_time) const override;

      /**
       * @copydoc IOcspResponse::raw_bits
       *
       * Forwards to Botan::OCSP::Response::raw_bits().
       */
      std::vector<std::uint8_t> raw_bits() const override;

   private:
      Botan::OCSP::Response response_;
};
