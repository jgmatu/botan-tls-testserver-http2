#pragma once

#include <string>
#include <string_view>

/// The request-target without query string or fragment.
std::string_view request_path(std::string_view target);

/// True when the path is absolute and has no ".." segment (no directory traversal).
bool is_safe_request_path(std::string_view path);

/// Joins the document root and the request path without doubling the slash.
std::string path_cat(std::string_view base, std::string_view path);

/// Maps a request path to a file below the document root; a trailing '/' means index.html.
std::string resolve_file_path(std::string_view document_root, std::string_view path);
