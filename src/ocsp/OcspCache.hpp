#pragma once

/**
 * @file
 * @brief Cached OCSP responses for stapling.
 * @ingroup ocsp
 */

#include "common/IClock.hpp"
#include "common/ILogger.hpp"
#include "ocsp/IOcspFetcher.hpp"
#include "ocsp/IOcspProvider.hpp"
#include "ocsp/OcspResponseStore.hpp"

#include <chrono>
#include <memory>

/**
 * @brief Fetches OCSP responses from the CA's responder and caches each one for as long as it is valid.
 *
 * Responses are keyed by the SHA-256 fingerprint of the server certificate.
 * A cache miss fetches synchronously through IOcspFetcher, on the thread that
 * runs the TLS handshake. Only a successful response whose status is good now
 * is stored and returned. The signature of the response is not checked here;
 * the client checks it.
 *
 * The storage is thread-safe. Two handshakes that miss at the same time may
 * both fetch; the last one stored wins.
 */
class OcspCache final : public IOcspProvider {
   public:
      /**
       * @param fetcher         Source of fresh responses.
       * @param clock           Time source for validity checks and expiry.
       * @param logger          Receives a message for every response that is cached.
       * @param cache_duration  The maximum duration an OCSP response is cached.
       */
      OcspCache(std::shared_ptr<IOcspFetcher> fetcher,
                std::shared_ptr<const IClock> clock,
                std::shared_ptr<ILogger> logger,
                std::chrono::minutes cache_duration);

      /**
       * @brief Returns the cached response, or fetches and caches a new one.
       *
       * The cache time comes from estimate_cache_duration().
       *
       * @param issuer  Certificate that issued @p subject (chain[1]).
       * @param subject Server certificate (chain[0]).
       * @return The DER-encoded response, or empty when the fetch failed or the status is not good.
       * @throws std::exception Whatever the fetcher throws on a network error.
       */
      std::vector<std::uint8_t> get_ocsp_response(const Botan::X509_Certificate& issuer,
                                                  const Botan::X509_Certificate& subject) override;

   private:
      std::shared_ptr<IOcspFetcher> fetcher_;
      std::shared_ptr<const IClock> clock_;
      std::shared_ptr<ILogger> logger_;
      OcspResponseStore store_;
      std::chrono::minutes cache_duration_;
};
