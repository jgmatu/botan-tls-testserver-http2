#include "ocsp/OcspCache.hpp"

#include "ocsp/OcspValidityEstimator.hpp"

OcspCache::OcspCache(std::shared_ptr<IOcspFetcher> fetcher,
                     std::shared_ptr<const IClock> clock,
                     std::shared_ptr<ILogger> logger,
                     std::chrono::minutes cache_duration) :
      fetcher_(std::move(fetcher)),
      clock_(clock),
      logger_(std::move(logger)),
      store_(std::move(clock)),
      cache_duration_(cache_duration) {}

std::vector<std::uint8_t> OcspCache::get_ocsp_response(const Botan::X509_Certificate& issuer,
                                                       const Botan::X509_Certificate& subject) {
   const auto fingerprint = subject.fingerprint("SHA-256");
   if(auto cached = store_.find(fingerprint); !cached.empty()) {
      return cached;
   }

   const auto response = fetcher_->fetch(issuer, subject);
   if(!response || !response->successful() ||
      response->status_for(issuer, subject, clock_->now()) != Botan::Certificate_Status_Code::OCSP_RESPONSE_GOOD) {
      return {};
   }

   // We do not explicitly check the signature of the OCSP response. Clients
   // will do that and reject if the response was counterfeit for some reason.

   const auto ref_time = clock_->now();
   const auto duration = estimate_cache_duration(*response, issuer, subject, ref_time, cache_duration_);
   logger_->info("caching OCSP response (" + std::to_string(duration.count()) + "min) for " + fingerprint);

   auto raw = response->raw_bits();
   store_.store(fingerprint, raw, ref_time + duration);
   return raw;
}
