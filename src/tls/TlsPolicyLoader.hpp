#pragma once

/**
 * @file
 * @brief Loading of the Botan TLS policy given on the command line.
 * @ingroup tls
 */

#include <botan/tls_policy.h>

#include <memory>
#include <string>

/**
 * @brief Loads the TLS policy selected by --policy.
 *
 * The files under policies/ use Botan's text policy format, for example
 * policies/tls_hybrid_pqc_browser.txt.
 *
 * @param policy_type "default" or empty for Botan's default policy; anything else is read as a policy file.
 * @return The policy.
 * @throws std::runtime_error When the file cannot be opened.
 * @throws Botan::Exception   When the file is not a valid text policy.
 */
std::shared_ptr<Botan::TLS::Policy> load_tls_policy(const std::string& policy_type);
