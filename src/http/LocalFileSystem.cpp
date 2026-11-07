#include "http/LocalFileSystem.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>

FileType LocalFileSystem::type(const std::string& path, std::error_code& ec) const {
   const auto status = std::filesystem::status(path, ec);
   if(ec == std::errc::no_such_file_or_directory) {
      ec.clear();
      return FileType::NotFound;
   }
   if(ec) {
      return FileType::Other;
   }
   return std::filesystem::is_regular_file(status) ? FileType::Regular : FileType::Other;
}

std::uintmax_t LocalFileSystem::size(const std::string& path, std::error_code& ec) const {
   return std::filesystem::file_size(path, ec);
}

bool LocalFileSystem::read_all(const std::string& path, std::string& out) const {
   std::ifstream input(path, std::ios::binary);
   if(!input) {
      return false;
   }
   out.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
   return !input.bad();
}
