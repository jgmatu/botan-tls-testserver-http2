#include "tls/AlpnSelector.hpp"

#include <gtest/gtest.h>

TEST(AlpnSelectorTest, ServerPreferenceWinsOverClientOrder) {
   EXPECT_EQ(select_alpn_protocol({"http/1.1", "h2"}, {"h2", "http/1.1"}), "h2");
   EXPECT_EQ(select_alpn_protocol({"h2", "http/1.1"}, {"http/1.1", "h2"}), "http/1.1");
}

TEST(AlpnSelectorTest, FallsBackToTheNextSharedProtocol) {
   EXPECT_EQ(select_alpn_protocol({"http/1.1"}, {"h2", "http/1.1"}), "http/1.1");
}

TEST(AlpnSelectorTest, NoSharedProtocolSelectsNothing) {
   EXPECT_EQ(select_alpn_protocol({"spdy/3"}, {"h2", "http/1.1"}), "");
   EXPECT_EQ(select_alpn_protocol({}, {"h2"}), "");
   EXPECT_EQ(select_alpn_protocol({"h2"}, {}), "");
}

TEST(AlpnSelectorTest, WebSocketPortOnlyOffersHttp11) {
   EXPECT_EQ(select_alpn_protocol({"h2", "http/1.1"}, {"http/1.1"}), "http/1.1");
   EXPECT_EQ(select_alpn_protocol({"h2"}, {"http/1.1"}), "");
}
