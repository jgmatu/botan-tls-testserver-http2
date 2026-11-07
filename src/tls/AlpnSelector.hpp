#pragma once

#include <string>
#include <vector>

/// First protocol of the server's preference list that the client also offers, or empty for none.
std::string select_alpn_protocol(const std::vector<std::string>& client_protocols,
                                 const std::vector<std::string>& server_preference);
