#include "tls/ConnectionDetails.hpp"

#include <gtest/gtest.h>

TEST(ConnectionDetailsTest, ProducesTheDocumentTheWebPageExpects) {
   ConnectionDetails details;
   details.kex_algo = "X25519MLKEM768";
   details.is_quantum_safe = true;
   details.wire_code = 0x11ec;
   details.websocket_port = 50443;

   EXPECT_EQ(to_json(details),
             R"({"kex_algo": "X25519MLKEM768","is_quantum_safe": true,"wire_code": "0x11ec","websocket_port": 50443})");
}

TEST(ConnectionDetailsTest, ClassicGroupIsNotQuantumSafe) {
   ConnectionDetails details;
   details.kex_algo = "x25519";
   details.is_quantum_safe = false;
   details.wire_code = 0x1d;
   details.websocket_port = 50444;

   EXPECT_EQ(to_json(details),
             R"({"kex_algo": "x25519","is_quantum_safe": false,"wire_code": "0x001d","websocket_port": 50444})");
}

TEST(ConnectionDetailsTest, WireCodeIsPaddedToFourHexDigitsAndPortStaysDecimal) {
   ConnectionDetails details;
   details.kex_algo = "unknown";
   details.wire_code = 0;
   details.websocket_port = 10;

   EXPECT_EQ(to_json(details),
             R"({"kex_algo": "unknown","is_quantum_safe": false,"wire_code": "0x0000","websocket_port": 10})");
}
