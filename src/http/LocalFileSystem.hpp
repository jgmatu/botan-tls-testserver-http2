#pragma once

/**
 * @file
 * @brief IFileSystem backed by std::filesystem.
 * @ingroup http
 */

#include "http/IFileSystem.hpp"

/**
 * @brief IFileSystem on the local disk, using std::filesystem and std::ifstream.
 *
 * Stateless, so one instance can be shared by every connection.
 */
class LocalFileSystem final : public IFileSystem {
   public:
      /**
       * @copydoc IFileSystem::type
       *
       * Uses std::filesystem::status(); no_such_file_or_directory becomes NotFound.
       */
      FileType type(const std::string& path, std::error_code& ec) const override;

      /**
       * @copydoc IFileSystem::size
       *
       * Uses std::filesystem::file_size().
       */
      std::uintmax_t size(const std::string& path, std::error_code& ec) const override;

      /**
       * @copydoc IFileSystem::read_all
       *
       * Reads the file in binary mode.
       */
      bool read_all(const std::string& path, std::string& out) const override;
};
