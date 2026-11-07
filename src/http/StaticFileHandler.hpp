#pragma once

#include "http/IFileSystem.hpp"
#include "http/IRequestHandler.hpp"

#include <memory>
#include <string>

/// Serves GET and HEAD for files below the document root.
class StaticFileHandler final : public IRequestHandler {
   public:
      StaticFileHandler(std::string document_root, std::shared_ptr<const IFileSystem> file_system, std::string server);

      void handle(const H2Request& request, IH2Responder& respond) override;

   private:
      std::string document_root_;
      std::shared_ptr<const IFileSystem> file_system_;
      std::string server_;
};
