#include "http/h2/H2ResponseHeaders.hpp"

#include "common/Strings.hpp"

bool is_content_length(std::string_view name) {
   return iequals(name, "content-length");
}

H2HeaderFields build_response_fields(int status, const std::vector<H2Header>& headers, std::size_t body_size) {
   bool has_length = false;
   for(const H2Header& header : headers) {
      if(is_content_length(header.name)) {
         has_length = true;
      }
   }

   H2HeaderFields fields;
   fields.emplace_back(":status", std::to_string(status));
   if(!has_length) {
      fields.emplace_back("content-length", std::to_string(body_size));
   }
   for(const H2Header& header : headers) {
      fields.emplace_back(lower_copy(header.name), header.value);
   }
   return fields;
}
