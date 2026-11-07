#pragma once

/**
 * @file
 * @brief Helper to submit a response with the headers common to every reply.
 * @ingroup http
 */

#include "http/h2/IH2Responder.hpp"

#include <string>
#include <vector>

/**
 * @brief Submits a response with content-type, server and any extra headers.
 *
 * The headers are sent in that order: content-type, server, then @p extra.
 * An empty @p body produces a response without DATA frames, which is how
 * HEAD is answered.
 *
 * @param respond      Responder of the stream being answered.
 * @param server       Value of the "server" header.
 * @param status       HTTP status code.
 * @param content_type Value of the "content-type" header.
 * @param body         Response body, may be empty.
 * @param extra        Additional headers. Names must be lowercase (HTTP/2).
 */
void reply(IH2Responder& respond,
           const std::string& server,
           int status,
           std::string content_type,
           std::string body,
           std::vector<H2Header> extra = {});
