#pragma once

/**
 * @file
 * @brief ASCII string helpers shared by the HTTP and WebSocket parsers.
 * @ingroup common
 */

#include <string>
#include <string_view>

/**
 * @brief Lowercases ASCII letters; other bytes are copied unchanged.
 * @param text Input text.
 * @return A lowercased copy of @p text.
 */
std::string lower_copy(std::string_view text);

/**
 * @brief Uppercases ASCII letters; other bytes are copied unchanged.
 * @param text Input text.
 * @return An uppercased copy of @p text.
 */
std::string upper_copy(std::string_view text);

/**
 * @brief Strips HTTP header whitespace.
 *
 * Removes leading spaces and tabs, and trailing spaces, tabs and CR. Inner
 * whitespace is kept.
 *
 * @param text Input text.
 * @return The trimmed copy.
 */
std::string trim_copy(std::string_view text);

/**
 * @brief ASCII case-insensitive equality.
 * @param left  First string.
 * @param right Second string.
 * @return True when both have the same length and match ignoring ASCII case.
 */
bool iequals(std::string_view left, std::string_view right);
