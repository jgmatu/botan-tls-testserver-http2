#include "ocsp/BotanOcspResponse.hpp"

bool BotanOcspResponse::successful() const {
   return response_.status() == Botan::OCSP::Response_Status_Code::Successful;
}

Botan::Certificate_Status_Code BotanOcspResponse::status_for(const Botan::X509_Certificate& issuer,
                                                             const Botan::X509_Certificate& subject,
                                                             std::chrono::system_clock::time_point ref_time) const {
   return response_.status_for(issuer, subject, ref_time);
}

std::vector<std::uint8_t> BotanOcspResponse::raw_bits() const {
   return response_.raw_bits();
}
