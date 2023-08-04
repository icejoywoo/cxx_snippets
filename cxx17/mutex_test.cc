#include <gtest/gtest.h>

#include <chrono>
#include <mutex>
#include <thread>

TEST(CxxTest, Mutex) {
    std::mutex mutex_;
    int count = 0;
    int iter = 10;

    std::vector<std::thread> threads;

    for (int i = 0; i < iter; i++) {
        std::thread t([&mutex_, &count]() {
            for (int i = 0; i < 10; i++) {
                std::lock_guard<std::mutex> guard(mutex_);
                count++;
                std::this_thread::sleep_for(std::chrono::nanoseconds(20));
            }
        });
        threads.push_back(std::move(t));
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(iter * 10, count);
}