#pragma once

/**
 * @file
 * @brief Read-only file system interface for the static file handler.
 * @ingroup http
 */

#include <cstdint>
#include <string>
#include <system_error>

/// @brief Kind of file found at a path.
enum class FileType {
   NotFound,  ///< Nothing exists at the path.
   Regular,   ///< A regular file that can be served.
   Other,     ///< A directory, device, socket or a path whose status failed.
};

/**
 * @brief Read-only file access used by StaticFileHandler.
 *
 * Exists so the handler can be tested without touching the disk. Paths are
 * already resolved against the document root when they get here.
 */
class IFileSystem {
   public:
      virtual ~IFileSystem() = default;

      /**
       * @brief Classifies the file at @p path.
       * @param path File path.
       * @param ec   Set on failure. A missing file is not a failure: it returns NotFound with @p ec clear.
       * @return The file type.
       */
      virtual FileType type(const std::string& path, std::error_code& ec) const = 0;

      /**
       * @brief Size of a regular file in bytes.
       * @param path File path.
       * @param ec   Set on failure.
       * @return The size, unspecified when @p ec is set.
       */
      virtual std::uintmax_t size(const std::string& path, std::error_code& ec) const = 0;

      /**
       * @brief Reads the whole file.
       * @param path File path.
       * @param out  Receives the file contents.
       * @return False when the file cannot be opened or read completely.
       */
      virtual bool read_all(const std::string& path, std::string& out) const = 0;
};
