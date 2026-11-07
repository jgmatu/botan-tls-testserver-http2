#pragma once

#include "http/h2/H2Message.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using H2HeaderFields = std::vector<std::pair<std::string, std::string>>;

bool is_content_length(std::string_view name);

/**
 * Header block of a response: :status first, then a content-length for the
 * body unless the caller supplied one, then the caller's headers with
 * lower-cased names (HTTP/2 forbids upper case).
 */
H2HeaderFields build_response_fields(int status, const std::vector<H2Header>& headers, std::size_t body_size);
