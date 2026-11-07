#include "common/Strings.hpp"

#include <cctype>

std::string lower_copy(std::string_view text) {
   std::string out(text);
   for(char& ch : out) {
      ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
   }
   return out;
}

std::string upper_copy(std::string_view text) {
   std::string out(text);
   for(char& ch : out) {
      ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
   }
   return out;
}

std::string trim_copy(std::string_view text) {
   while(!text.empty() && (text.front() == ' ' || text.front() == '\t')) {
      text.remove_prefix(1);
   }
   while(!text.empty() && (text.back() == ' ' || text.back() == '\t' || text.back() == '\r')) {
      text.remove_suffix(1);
   }
   return std::string(text);
}

bool iequals(std::string_view left, std::string_view right) {
   if(left.size() != right.size()) {
      return false;
   }
   for(std::size_t i = 0; i < left.size(); ++i) {
      const auto a = static_cast<unsigned char>(left[i]);
      const auto b = static_cast<unsigned char>(right[i]);
      if(std::tolower(a) != std::tolower(b)) {
         return false;
      }
   }
   return true;
}
