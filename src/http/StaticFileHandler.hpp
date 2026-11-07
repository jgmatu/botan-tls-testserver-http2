#pragma once

/**
 * @file
 * @brief Static file serving from the document root.
 * @ingroup http
 */

#include "http/IFileSystem.hpp"
#include "http/IRequestHandler.hpp"

#include <memory>
#include <string>

/**
 * @brief Serves GET and HEAD for files below the document root.
 *
 * A path ending in '/' serves index.html from that directory. The content
 * type comes from mime_type(). HEAD answers with content-length and no body.
 *
 * Errors are answered with a small text/html body:
 * - 400 for any method other than GET or HEAD, or for a path that is not
 *   absolute or contains ".." (see is_safe_request_path());
 * - 404 when the file does not exist, with the path as body;
 * - 500 when the path is not a regular file or cannot be stat'ed or read.
 *
 * The whole file is read into memory for each request.
 */
class StaticFileHandler final : public IRequestHandler {
   public:
      /**
       * @param document_root Directory that request paths are resolved against.
       * @param file_system   File access, a LocalFileSystem in production and a mock in tests.
       * @param server        Value of the "server" response header.
       */
      StaticFileHandler(std::string document_root, std::shared_ptr<const IFileSystem> file_system, std::string server);

      /// @copydoc IRequestHandler::handle
      void handle(const H2Request& request, IH2Responder& respond) override;

   private:
      std::string document_root_;
      std::shared_ptr<const IFileSystem> file_system_;
      std::string server_;
};
