#pragma once

#include "http/IFileSystem.hpp"

#include <gmock/gmock.h>

class MockFileSystem : public IFileSystem {
   public:
      MOCK_METHOD(FileType, type, (const std::string& path, std::error_code& ec), (const, override));
      MOCK_METHOD(std::uintmax_t, size, (const std::string& path, std::error_code& ec), (const, override));
      MOCK_METHOD(bool, read_all, (const std::string& path, std::string& out), (const, override));
};
