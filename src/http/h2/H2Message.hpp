#pragma once

/**
 * @file
 * @brief Plain data types for HTTP/2 requests and headers.
 * @ingroup h2
 */

#include <cstdint>
#include <string>
#include <vector>

/// @brief One header field, as received or as to be sent.
struct H2Header {
      std::string name;   ///< Header name. HTTP/2 requires lowercase; responses are lowercased on submit.
      std::string value;  ///< Header value.
};

/**
 * @brief A complete request, as handed to IH2Application::on_request().
 *
 * Built by H2RequestAssembler from the HEADERS and DATA frames of one stream.
 */
struct H2Request {
      int32_t stream_id = 0;         ///< HTTP/2 stream the request arrived on.
      std::string method;            ///< Value of the :method pseudo-header.
      std::string path;              ///< Value of the :path pseudo-header, query string included.
      std::vector<H2Header> headers; ///< Regular headers in arrival order. Other pseudo-headers are dropped.
      std::string body;              ///< Concatenated DATA payload.
};
