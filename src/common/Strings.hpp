#pragma once

#include <string>
#include <string_view>

std::string lower_copy(std::string_view text);

std::string upper_copy(std::string_view text);

/// Strips leading spaces/tabs and trailing spaces/tabs/CR (HTTP header whitespace).
std::string trim_copy(std::string_view text);

/// ASCII case-insensitive comparison.
bool iequals(std::string_view left, std::string_view right);
