#include "ocsp/BeastOcspFetcher.hpp"

#include "ocsp/BotanOcspResponse.hpp"
#include "ocsp/Url.hpp"

#include <botan/ocsp.h>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

BeastOcspFetcher::BeastOcspFetcher(std::chrono::seconds request_timeout, std::shared_ptr<ILogger> logger) :
      request_timeout_(request_timeout), logger_(std::move(logger)) {}

std::unique_ptr<IOcspResponse> BeastOcspFetcher::fetch(const Botan::X509_Certificate& issuer,
                                                       const Botan::X509_Certificate& subject) {
   const auto responder = subject.ocsp_responder();
   logger_->info("going to fetch OCSP response from " + responder);

   if(responder.empty()) {
      return nullptr;
   }

   const auto url = parse_url(responder);
   if(!url.has_value()) {
      return nullptr;
   }

   const Botan::OCSP::Request req(issuer, subject);

   namespace http = boost::beast::http;
   namespace asio = boost::asio;
   using tcp = boost::asio::ip::tcp;

   // Create an io_context
   asio::io_context ioc;

   // Lookup the domain name
   tcp::resolver resolver(ioc);
   const auto results = resolver.resolve(url->host, url->port.value_or(url->protocol == "http" ? "80" : "443"));

   // Create a TCP stream
   boost::beast::tcp_stream socket(ioc);
   socket.expires_after(request_timeout_);
   socket.connect(results);

   // Create an HTTP request
   http::request<http::vector_body<uint8_t>> http_request(http::verb::post, url->target, 10);
   http_request.set(http::field::host, url->host);
   http_request.set(http::field::user_agent, "Botan OCSP Client");
   http_request.set(http::field::content_type, "application/ocsp-request");
   http_request.set(http::field::accept, "application/ocsp-response");
   http_request.body() = req.BER_encode();
   http_request.prepare_payload();

   // Send the HTTP request
   http::write(socket, http_request);

   // Receive the HTTP response
   boost::beast::flat_buffer buffer;
   http::response<http::vector_body<uint8_t>> response;
   http::read(socket, buffer, response);

   // Check if the response is successful
   if(response.result() != http::status::ok) {
      return nullptr;
   }

   // Process the response and return the OCSP response
   return std::make_unique<BotanOcspResponse>(Botan::OCSP::Response(response.body()));
}
