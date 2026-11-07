#pragma once

#include "http/IFileSystem.hpp"

class LocalFileSystem final : public IFileSystem {
   public:
      FileType type(const std::string& path, std::error_code& ec) const override;

      std::uintmax_t size(const std::string& path, std::error_code& ec) const override;

      bool read_all(const std::string& path, std::string& out) const override;
};
