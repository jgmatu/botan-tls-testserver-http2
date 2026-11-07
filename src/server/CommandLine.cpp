#include "server/CommandLine.hpp"

#include <boost/program_options.hpp>

#include <sstream>

CommandLineResult parse_command_line(int argc, const char* const argv[]) {
   namespace po = boost::program_options;

   // clang-format off
   po::options_description desc("Allowed options");
   desc.add_options()
      ("help", "produce help message")
      ("port", po::value<uint16_t>()->required(), "HTTP/2 listen port")
      ("ws-port", po::value<uint16_t>()->default_value(50444), "WebSocket (WSS) listen port")
      ("policy", po::value<std::string>()->default_value("default"), "Botan policy file (default: Botan's default policy)")
      ("cert", po::value<std::string>()->required(), "Path to the server's certificate chain")
      ("key", po::value<std::string>()->required(), "Path to the server's certificate private key file")
      ("ocsp-request-timeout", po::value<uint64_t>()->default_value(10), "OCSP request timeout in seconds")
      ("ocsp-cache-time", po::value<uint64_t>()->default_value(6 * 60), "Cache validity time for OCSP responses in minutes")
      ("document-root", po::value<std::string>()->default_value("webroot"), "Path to the server's static documents folder");
   // clang-format on

   CommandLineResult result;
   po::variables_map vm;
   try {
      po::store(po::parse_command_line(argc, argv, desc), vm);

      if(vm.count("help") != 0 || argc < 2) {
         std::ostringstream usage;
         usage << "Usage: \n" << desc;
         result.status = CommandLineResult::Status::Help;
         result.message = usage.str();
         return result;
      }

      po::notify(vm);
   } catch(const std::exception& ex) {
      result.status = CommandLineResult::Status::Error;
      result.message = ex.what();
      return result;
   }

   ServerOptions& options = result.options;
   options.port = vm["port"].as<uint16_t>();
   options.ws_port = vm["ws-port"].as<uint16_t>();
   options.policy = vm["policy"].as<std::string>();
   options.cert = vm["cert"].as<std::string>();
   options.key = vm["key"].as<std::string>();
   options.ocsp_request_timeout_seconds = vm["ocsp-request-timeout"].as<uint64_t>();
   options.ocsp_cache_minutes = vm["ocsp-cache-time"].as<uint64_t>();
   options.document_root = vm["document-root"].as<std::string>();
   return result;
}
