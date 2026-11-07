#include "ocsp/OcspResponseStore.hpp"

#include <mutex>

OcspResponseStore::OcspResponseStore(std::shared_ptr<const IClock> clock) : clock_(std::move(clock)) {}

std::vector<std::uint8_t> OcspResponseStore::find(const std::string& fingerprint) const {
   std::shared_lock lock(mutex_);

   const auto entry = entries_.find(fingerprint);
   if(entry == entries_.end() || entry->second.valid_until < clock_->now()) {
      // We don't evict the entry here, as we don't want to obtain a write lock here
      return {};
   }

   return entry->second.response;
}

void OcspResponseStore::store(const std::string& fingerprint,
                              std::vector<std::uint8_t> response,
                              std::chrono::system_clock::time_point valid_until) {
   std::unique_lock lock(mutex_);
   entries_[fingerprint] = Entry{std::move(response), valid_until};
}
