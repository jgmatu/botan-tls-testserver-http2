#pragma once

/**
 * @file
 * @brief Estimate of how long an OCSP response can be cached.
 * @ingroup ocsp
 */

#include "ocsp/IOcspResponse.hpp"

#include <botan/x509cert.h>

#include <chrono>

/**
 * @brief Guesses how long an OCSP response stays usable.
 *
 * Botan does not expose an OCSP response's validity period and responses
 * don't always carry 'next_update', so the usable duration is guessed:
 * start at @p max_duration and halve it while the response would not yet
 * be valid at the end of that period.
 *
 * @param response     Response to evaluate.
 * @param issuer       Certificate that issued @p subject.
 * @param subject      Certificate the response is about.
 * @param ref_time     Start of the cache period, normally now.
 * @param max_duration Upper bound, from --ocsp-cache-time.
 * @return A duration between zero and @p max_duration.
 */
std::chrono::minutes estimate_cache_duration(const IOcspResponse& response,
                                             const Botan::X509_Certificate& issuer,
                                             const Botan::X509_Certificate& subject,
                                             std::chrono::system_clock::time_point ref_time,
                                             std::chrono::minutes max_duration);
