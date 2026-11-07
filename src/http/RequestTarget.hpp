#pragma once

/**
 * @file
 * @brief Parsing of the request-target and mapping to files.
 * @ingroup http
 */

#include <string>
#include <string_view>

/**
 * @brief The request-target without query string or fragment.
 * @param target Value of the :path pseudo-header.
 * @return The part before the first '?' or '#'. It views into @p target.
 */
std::string_view request_path(std::string_view target);

/**
 * @brief Rejects paths that could escape the document root.
 * @param path Request path without query string.
 * @return True when @p path starts with '/' and contains no "..".
 */
bool is_safe_request_path(std::string_view path);

/**
 * @brief Joins the document root and the request path without doubling the slash.
 * @param base Document root. When empty, @p path is returned unchanged.
 * @param path Absolute request path.
 * @return @p base without a trailing '/' followed by @p path.
 */
std::string path_cat(std::string_view base, std::string_view path);

/**
 * @brief Maps a request path to a file below the document root.
 *
 * A trailing '/' maps to index.html in that directory. The path is not
 * checked here; call is_safe_request_path() first.
 *
 * @param document_root Directory that is served.
 * @param path          Absolute request path.
 * @return The file path.
 */
std::string resolve_file_path(std::string_view document_root, std::string_view path);
