#pragma once

#include <cstdint>
#include <string>
#include <system_error>

enum class FileType {
   NotFound,
   Regular,
   Other,
};

/// Read-only file access used by the static file handler.
class IFileSystem {
   public:
      virtual ~IFileSystem() = default;

      /// NotFound is reported without an error; any other failure sets @p ec.
      virtual FileType type(const std::string& path, std::error_code& ec) const = 0;

      virtual std::uintmax_t size(const std::string& path, std::error_code& ec) const = 0;

      /// Reads the whole file. Returns false when it cannot be read completely.
      virtual bool read_all(const std::string& path, std::string& out) const = 0;
};
