#include "ocsp/OcspValidityEstimator.hpp"

std::chrono::minutes estimate_cache_duration(const IOcspResponse& response,
                                             const Botan::X509_Certificate& issuer,
                                             const Botan::X509_Certificate& subject,
                                             std::chrono::system_clock::time_point ref_time,
                                             std::chrono::minutes max_duration) {
   auto cache_duration = max_duration;
   while(cache_duration != std::chrono::minutes::zero() &&
         response.status_for(issuer, subject, ref_time + cache_duration) ==
            Botan::Certificate_Status_Code::OCSP_NOT_YET_VALID) {
      cache_duration /= 2;
   }
   return cache_duration;
}
