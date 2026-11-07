#pragma once

#include "ocsp/IOcspResponse.hpp"

#include <botan/ocsp.h>

class BotanOcspResponse final : public IOcspResponse {
   public:
      explicit BotanOcspResponse(Botan::OCSP::Response response) : response_(std::move(response)) {}

      bool successful() const override;

      Botan::Certificate_Status_Code status_for(const Botan::X509_Certificate& issuer,
                                                const Botan::X509_Certificate& subject,
                                                std::chrono::system_clock::time_point ref_time) const override;

      std::vector<std::uint8_t> raw_bits() const override;

   private:
      Botan::OCSP::Response response_;
};
