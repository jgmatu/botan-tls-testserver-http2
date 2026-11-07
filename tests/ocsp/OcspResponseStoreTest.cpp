#include "ocsp/OcspResponseStore.hpp"

#include "mocks/MockClock.hpp"

#include <gtest/gtest.h>

#include <thread>
#include <vector>

using namespace std::chrono_literals;

class OcspResponseStoreTest : public ::testing::Test {
   protected:
      std::shared_ptr<FakeClock> clock = std::make_shared<FakeClock>();
      OcspResponseStore store{clock};
};

TEST_F(OcspResponseStoreTest, UnknownFingerprintIsEmpty) {
   EXPECT_TRUE(store.find("nope").empty());
}

TEST_F(OcspResponseStoreTest, StoredResponseIsReturnedUntilItExpires) {
   store.store("fp", {1, 2, 3}, clock->now() + 10min);

   EXPECT_EQ(store.find("fp"), (std::vector<std::uint8_t>{1, 2, 3}));

   clock->advance(9min);
   EXPECT_EQ(store.find("fp"), (std::vector<std::uint8_t>{1, 2, 3}));

   clock->advance(2min);
   EXPECT_TRUE(store.find("fp").empty());
}

TEST_F(OcspResponseStoreTest, EntriesAreIndependent) {
   store.store("a", {1}, clock->now() + 1min);
   store.store("b", {2}, clock->now() + 1h);

   clock->advance(5min);

   EXPECT_TRUE(store.find("a").empty());
   EXPECT_EQ(store.find("b"), std::vector<std::uint8_t>{2});
}

TEST_F(OcspResponseStoreTest, StoringAgainReplacesTheEntry) {
   store.store("fp", {1}, clock->now() + 1min);
   store.store("fp", {9, 9}, clock->now() + 1h);

   clock->advance(5min);

   EXPECT_EQ(store.find("fp"), (std::vector<std::uint8_t>{9, 9}));
}

TEST_F(OcspResponseStoreTest, ConcurrentReadersAndWritersDoNotCorruptTheStore) {
   store.store("fp", {7}, clock->now() + 1h);

   std::vector<std::thread> threads;
   for(int t = 0; t < 4; ++t) {
      threads.emplace_back([this, t] {
         for(int i = 0; i < 500; ++i) {
            if(t == 0) {
               store.store("fp", {7}, clock->now() + 1h);
            } else {
               EXPECT_EQ(store.find("fp"), std::vector<std::uint8_t>{7});
            }
         }
      });
   }
   for(auto& thread : threads) {
      thread.join();
   }
}
