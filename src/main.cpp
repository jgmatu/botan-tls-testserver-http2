/**
 * Botan post-quantum TLS test server
 *
 * (C) 2023 René Fischer, René Meusel
 *
 * HTTP/2 is framed with nghttp2 on top of Botan::TLS::Stream. A second thread
 * serves a classic WebSocket (RFC 6455) over the same TLS stack.
 */

#include "common/StderrLogger.hpp"
#include "server/CommandLine.hpp"
#include "server/ServerApp.hpp"

#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
   const CommandLineResult command_line = parse_command_line(argc, argv);

   switch(command_line.status) {
      case CommandLineResult::Status::Help:
         std::cerr << command_line.message << std::endl;
         return 0;
      case CommandLineResult::Status::Error:
         std::cerr << command_line.message << std::endl;
         return 1;
      case CommandLineResult::Status::Run:
         break;
   }

   ServerApp app(command_line.options, std::make_shared<StderrLogger>());
   return app.run();
}
