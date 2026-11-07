#pragma once

#include <botan/tls_policy.h>

#include <memory>
#include <string>

/// "default" (or empty) gives Botan's default policy; anything else is read as a policy file.
/// Throws std::runtime_error when the file cannot be opened.
std::shared_ptr<Botan::TLS::Policy> load_tls_policy(const std::string& policy_type);
