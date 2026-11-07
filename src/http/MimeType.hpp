#pragma once

#include <string>
#include <string_view>

/// Content type for a file path, chosen by its extension.
std::string mime_type(std::string_view path);
