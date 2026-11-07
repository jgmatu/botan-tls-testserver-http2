#include "http/RequestTarget.hpp"

std::string_view request_path(std::string_view target) {
   const auto cut = target.find_first_of("?#");
   if(cut == std::string_view::npos) {
      return target;
   }
   return target.substr(0, cut);
}

bool is_safe_request_path(std::string_view path) {
   return !path.empty() && path.front() == '/' && path.find("..") == std::string_view::npos;
}

std::string path_cat(std::string_view base, std::string_view path) {
   if(base.empty()) {
      return std::string(path);
   }

   std::string result(base);
   if(result.back() == '/') {
      result.pop_back();
   }
   result.append(path);
   return result;
}

std::string resolve_file_path(std::string_view document_root, std::string_view path) {
   std::string file = path_cat(document_root, path);
   if(!path.empty() && path.back() == '/') {
      file.append("index.html");
   }
   return file;
}
