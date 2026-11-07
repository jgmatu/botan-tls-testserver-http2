#pragma once

#include <botan/data_src.h>
#include <botan/x509cert.h>

#include <string>

/// Certificates generated once with the botan CLI (see tests/fixtures).
inline Botan::X509_Certificate load_fixture_certificate(const std::string& file) {
   return Botan::X509_Certificate(std::string(TEST_FIXTURE_DIR) + "/" + file);
}
