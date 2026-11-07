#pragma once

/**
 * @file
 * @brief Content type lookup by file extension.
 * @ingroup http
 */

#include <string>
#include <string_view>

/**
 * @brief Content type for a file path, chosen by its extension.
 *
 * The extension is compared ignoring ASCII case. Known extensions are html,
 * htm, css, txt, js, json, png, ico (served as image/png), jpe, jpeg, jpg,
 * gif, svg and svgz.
 *
 * @param path File path or request path.
 * @return The content type, or "application/text" for an unknown extension.
 */
std::string mime_type(std::string_view path);
