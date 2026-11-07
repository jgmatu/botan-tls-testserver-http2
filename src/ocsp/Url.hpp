#pragma once

#include <optional>
#include <string>

struct Url {
      std::string protocol;
      std::string host;
      std::optional<std::string> port;
      std::string target;
};

/// Splits "scheme://host[:port][/target]". Nothing when there is no "://".
/// The protocol is only filled ("http") when the URL carries no explicit port.
std::optional<Url> parse_url(const std::string& url);
