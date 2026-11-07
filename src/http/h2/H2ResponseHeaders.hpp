#pragma once

/**
 * @file
 * @brief Construction of the HTTP/2 response header block.
 * @ingroup h2
 */

#include "http/h2/H2Message.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/// Header block as name/value pairs, in the order they are sent.
using H2HeaderFields = std::vector<std::pair<std::string, std::string>>;

/**
 * @brief Whether a header name is content-length.
 * @param name Header name in any case.
 * @return True for "content-length", ignoring ASCII case.
 */
bool is_content_length(std::string_view name);

/**
 * @brief Header block of a response.
 *
 * The order is :status first, then content-length for the body unless the
 * caller supplied one, then the caller's headers with lowercased names
 * (HTTP/2 forbids uppercase). A caller-supplied content-length is how HEAD
 * announces the size of a body it does not send.
 *
 * @param status    HTTP status code.
 * @param headers   Caller's headers.
 * @param body_size Size of the body that will be sent.
 * @return The fields to pass to nghttp2.
 */
H2HeaderFields build_response_fields(int status, const std::vector<H2Header>& headers, std::size_t body_size);
