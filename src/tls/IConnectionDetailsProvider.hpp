#pragma once

#include <string>

/// Describes the TLS connection of the current client (key exchange group, ...).
class IConnectionDetailsProvider {
   public:
      virtual ~IConnectionDetailsProvider() = default;

      virtual std::string connection_details_json() const = 0;
};
