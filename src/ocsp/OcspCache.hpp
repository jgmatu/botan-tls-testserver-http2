#pragma once

#include "common/IClock.hpp"
#include "common/ILogger.hpp"
#include "ocsp/IOcspFetcher.hpp"
#include "ocsp/IOcspProvider.hpp"
#include "ocsp/OcspResponseStore.hpp"

#include <chrono>
#include <memory>

/**
 * Fetches OCSP responses from the CA's responder and pins them to the
 * server's certificate, caching each one for as long as it is valid.
 */
class OcspCache final : public IOcspProvider {
   public:
      /**
       * @param cache_duration  The maximum duration an OCSP response is cached.
       */
      OcspCache(std::shared_ptr<IOcspFetcher> fetcher,
                std::shared_ptr<const IClock> clock,
                std::shared_ptr<ILogger> logger,
                std::chrono::minutes cache_duration);

      std::vector<std::uint8_t> get_ocsp_response(const Botan::X509_Certificate& issuer,
                                                  const Botan::X509_Certificate& subject) override;

   private:
      std::shared_ptr<IOcspFetcher> fetcher_;
      std::shared_ptr<const IClock> clock_;
      std::shared_ptr<ILogger> logger_;
      OcspResponseStore store_;
      std::chrono::minutes cache_duration_;
};
