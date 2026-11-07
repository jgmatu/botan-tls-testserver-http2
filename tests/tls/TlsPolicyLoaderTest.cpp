#include "tls/TlsPolicyLoader.hpp"

#include <botan/tls_policy.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>

TEST(TlsPolicyLoaderTest, DefaultAndEmptyGiveBotansDefaultPolicy) {
   const auto by_name = load_tls_policy("default");
   const auto empty = load_tls_policy("");

   ASSERT_TRUE(by_name);
   ASSERT_TRUE(empty);
   EXPECT_EQ(dynamic_cast<Botan::TLS::Text_Policy*>(by_name.get()), nullptr);
   EXPECT_EQ(dynamic_cast<Botan::TLS::Text_Policy*>(empty.get()), nullptr);
}

TEST(TlsPolicyLoaderTest, UnknownNameThatIsNoFileThrows) {
   EXPECT_THROW(load_tls_policy("/definitely/not/a/policy.txt"), std::runtime_error);
}

TEST(TlsPolicyLoaderTest, PolicyFileIsLoadedAsTextPolicy) {
   const auto file = std::filesystem::temp_directory_path() / "testserver_policy_test.txt";
   {
      std::ofstream out(file);
      out << "allow_tls12 = false\n";
   }

   const auto policy = load_tls_policy(file.string());
   std::filesystem::remove(file);

   ASSERT_TRUE(policy);
   EXPECT_NE(dynamic_cast<Botan::TLS::Text_Policy*>(policy.get()), nullptr);
   EXPECT_FALSE(policy->allow_tls12());
}

TEST(TlsPolicyLoaderTest, ShippedPoliciesParse) {
   for(const char* name : {"pqc_basic.txt", "pqc_all.txt"}) {
      const std::string path = std::string(TEST_FIXTURE_DIR) + "/../../policies/" + name;
      EXPECT_NO_THROW(load_tls_policy(path)) << path;
   }
}
