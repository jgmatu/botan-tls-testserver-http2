#pragma once

#include "ocsp/IOcspResponse.hpp"

#include <botan/x509cert.h>

#include <chrono>

/**
 * Botan does not expose an OCSP response's validity period and responses
 * don't always carry 'next_update', so the usable duration is guessed:
 * start at @p max_duration and halve it while the response would not yet
 * be valid at the end of that period.
 */
std::chrono::minutes estimate_cache_duration(const IOcspResponse& response,
                                             const Botan::X509_Certificate& issuer,
                                             const Botan::X509_Certificate& subject,
                                             std::chrono::system_clock::time_point ref_time,
                                             std::chrono::minutes max_duration);
