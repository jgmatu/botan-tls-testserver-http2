#pragma once

#include "http/h2/IH2Responder.hpp"

#include <string>
#include <vector>

/// Submits a response carrying the content-type and server headers plus any extra ones.
void reply(IH2Responder& respond,
           const std::string& server,
           int status,
           std::string content_type,
           std::string body,
           std::vector<H2Header> extra = {});
