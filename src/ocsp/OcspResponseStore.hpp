#pragma once

#include "common/IClock.hpp"

#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

/// Thread-safe map from a certificate fingerprint to a raw OCSP response with an expiry time.
class OcspResponseStore {
   public:
      explicit OcspResponseStore(std::shared_ptr<const IClock> clock);

      /// The stored response, or empty if there is none or it has expired.
      std::vector<std::uint8_t> find(const std::string& fingerprint) const;

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
