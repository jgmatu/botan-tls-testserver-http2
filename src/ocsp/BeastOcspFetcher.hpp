#pragma once

#include "common/ILogger.hpp"
#include "ocsp/IOcspFetcher.hpp"

#include <chrono>
#include <memory>

/// Plain HTTP OCSP client built on Boost.Beast.
class BeastOcspFetcher final : public IOcspFetcher {
   public:
      BeastOcspFetcher(std::chrono::seconds request_timeout, std::shared_ptr<ILogger> logger);

      std::unique_ptr<IOcspResponse> fetch(const Botan::X509_Certificate& issuer,
                                           const Botan::X509_Certificate& subject) override;

   private:
      std::chrono::seconds request_timeout_;
      std::shared_ptr<ILogger> logger_;
};
