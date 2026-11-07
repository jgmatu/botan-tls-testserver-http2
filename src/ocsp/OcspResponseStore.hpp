#pragma once

/**
 * @file
 * @brief Thread-safe storage of OCSP responses with an expiry time.
 * @ingroup ocsp
 */

#include "common/IClock.hpp"

#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

/**
 * @brief Thread-safe map from a certificate fingerprint to a raw OCSP response with an expiry time.
 *
 * Lookups take a shared lock and stores take an exclusive lock. Expired
 * entries are not evicted, only ignored, until a new response for the same
 * fingerprint overwrites them.
 */
class OcspResponseStore {
   public:
      /// @param clock Time source used to decide whether an entry has expired.
      explicit OcspResponseStore(std::shared_ptr<const IClock> clock);

      /**
       * @brief Looks up a response.
       * @param fingerprint Certificate fingerprint used as key.
       * @return The stored response, or empty if there is none or it has expired.
       */
      std::vector<std::uint8_t> find(const std::string& fingerprint) const;

      /**
       * @brief Stores or replaces a response.
       * @param fingerprint Certificate fingerprint used as key.
       * @param response    DER-encoded OCSP response.
       * @param valid_until Time after which find() no longer returns it.
       */
      void store(const std::string& fingerprint,
                 std::vector<std::uint8_t> response,
                 std::chrono::system_clock::time_point valid_until);

   private:
      struct Entry {
            std::vector<std::uint8_t> response;
            std::chrono::system_clock::time_point valid_until;
      };

      std::shared_ptr<const IClock> clock_;
      mutable std::shared_mutex mutex_;
      std::map<std::string, Entry> entries_;
};
