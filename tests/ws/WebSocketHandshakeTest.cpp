#include "ws/WebSocketHandshake.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>
#include <utility>

namespace {

const std::string kValidHead =
   "GET /ws HTTP/1.1\r\n"
   "Host: localhost:50443\r\n"
   "Upgrade: websocket\r\n"
   "Connection: keep-alive, Upgrade\r\n"
   "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
   "Sec-WebSocket-Version: 13";

}  // namespace

TEST(WebSocketHandshakeTest, AcceptKeyMatchesTheRfc6455Example) {
   EXPECT_EQ(accept_key("dGhlIHNhbXBsZSBub25jZQ=="), "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=");
}

TEST(WebSocketHandshakeTest, ParsesRequestLineAndRelevantHeaders) {
   const HandshakeRequest request = parse_handshake_request(kValidHead);

   EXPECT_EQ(request.method, "GET");
   EXPECT_EQ(request.target, "/ws");
   EXPECT_EQ(request.version, "HTTP/1.1");
   EXPECT_EQ(request.upgrade, "websocket");
   EXPECT_EQ(request.connection, "keep-alive, Upgrade");
   EXPECT_EQ(request.key, "dGhlIHNhbXBsZSBub25jZQ==");
   EXPECT_EQ(request.ws_version, "13");
}

TEST(WebSocketHandshakeTest, HeaderNamesAreCaseInsensitiveAndValuesTrimmed) {
   const HandshakeRequest request = parse_handshake_request(
      "GET /ws HTTP/1.1\r\nUPGRADE:   websocket  \r\nconnection:Upgrade\r\nSEC-WEBSOCKET-KEY:\tabc\t\r\n"
      "sec-websocket-version: 13");

   EXPECT_EQ(request.upgrade, "websocket");
   EXPECT_EQ(request.connection, "Upgrade");
   EXPECT_EQ(request.key, "abc");
   EXPECT_EQ(request.ws_version, "13");
}

TEST(WebSocketHandshakeTest, MalformedRequestLineLeavesFieldsEmpty) {
   const HandshakeRequest request = parse_handshake_request("GARBAGE\r\nUpgrade: websocket");

   EXPECT_TRUE(request.method.empty());
   EXPECT_TRUE(request.target.empty());
   EXPECT_TRUE(request.version.empty());
   EXPECT_EQ(request.upgrade, "websocket");
}

TEST(WebSocketHandshakeTest, LinesWithoutColonAreSkipped) {
   const HandshakeRequest request = parse_handshake_request("GET /ws HTTP/1.1\r\nnot a header\r\nUpgrade: websocket");

   EXPECT_EQ(request.upgrade, "websocket");
}

TEST(WebSocketHandshakeTest, HeaderTokenListMatching) {
   EXPECT_TRUE(header_has_token("Upgrade", "upgrade"));
   EXPECT_TRUE(header_has_token("keep-alive, Upgrade", "upgrade"));
   EXPECT_TRUE(header_has_token(" upgrade ,close", "upgrade"));
   EXPECT_FALSE(header_has_token("keep-alive", "upgrade"));
   EXPECT_FALSE(header_has_token("upgrades", "upgrade"));
   EXPECT_FALSE(header_has_token("", "upgrade"));
}

TEST(WebSocketHandshakeTest, ValidRequestIsAccepted) {
   EXPECT_TRUE(is_valid_upgrade_request(parse_handshake_request(kValidHead)));
}

TEST(WebSocketHandshakeTest, QueryStringOnThePathIsAccepted) {
   HandshakeRequest request = parse_handshake_request(kValidHead);
   request.target = "/ws?token=1";

   EXPECT_TRUE(is_valid_upgrade_request(request));
}

TEST(WebSocketHandshakeTest, Http10IsAccepted) {
   HandshakeRequest request = parse_handshake_request(kValidHead);
   request.version = "HTTP/1.0";

   EXPECT_TRUE(is_valid_upgrade_request(request));
}

TEST(WebSocketHandshakeTest, InvalidRequestsAreRejected) {
   const std::pair<const char*, std::function<void(HandshakeRequest&)>> cases[] = {
      {"POST", [](HandshakeRequest& r) { r.method = "POST"; }},
      {"wrong path", [](HandshakeRequest& r) { r.target = "/chat"; }},
      {"HTTP/2", [](HandshakeRequest& r) { r.version = "HTTP/2"; }},
      {"no upgrade header", [](HandshakeRequest& r) { r.upgrade.clear(); }},
      {"no connection upgrade", [](HandshakeRequest& r) { r.connection = "keep-alive"; }},
      {"no key", [](HandshakeRequest& r) { r.key.clear(); }},
      {"old websocket version", [](HandshakeRequest& r) { r.ws_version = "8"; }},
   };

   for(const auto& [name, mutate] : cases) {
      HandshakeRequest request = parse_handshake_request(kValidHead);
      ASSERT_TRUE(is_valid_upgrade_request(request));

      mutate(request);

      EXPECT_FALSE(is_valid_upgrade_request(request)) << name;
   }
}

TEST(WebSocketHandshakeTest, SwitchingProtocolsResponseCarriesTheAcceptKey) {
   const std::string response = switching_protocols_response("dGhlIHNhbXBsZSBub25jZQ==");

   EXPECT_EQ(response,
             "HTTP/1.1 101 Switching Protocols\r\n"
             "Upgrade: websocket\r\n"
             "Connection: Upgrade\r\n"
             "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n\r\n");
}

TEST(WebSocketHandshakeTest, BadRequestResponseClosesTheConnection) {
   EXPECT_THAT(std::string(kBadRequestResponse), ::testing::StartsWith("HTTP/1.1 400 Bad Request\r\n"));
   EXPECT_THAT(std::string(kBadRequestResponse), ::testing::HasSubstr("Connection: close"));
   EXPECT_THAT(std::string(kBadRequestResponse), ::testing::EndsWith("\r\n\r\n"));
}
