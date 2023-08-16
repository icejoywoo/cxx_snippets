#include <gtest/gtest.h>

#include "folly/executors/ThreadedExecutor.h"
#include "folly/futures/Future.h"

/// https://github.com/facebook/folly/blob/main/folly/docs/Futures.md
/// https://engineering.fb.com/2015/06/19/developer-tools/futures-for-c-11-at-facebook/
/// Futures is a framework for expressing asynchronous code in C++ using the Promise/Future pattern.
TEST(FutureTest, Basic) {
    {
        folly::Promise<int> p;
        folly::Future<int> f = p.getFuture();

        bool flag = false;
        folly::Future<folly::Unit> f2 = std::move(f).thenValue([&flag](int x) {
            std::cout << "f(" << x << ")" << std::endl;
            flag = true;
            return;
        });

        p.setValue(5);

        std::move(f2).thenValue([flag](folly::Unit /* not used */) {
            EXPECT_TRUE(flag);
        });
    }

    {
        folly::ThreadedExecutor executor;
        folly::Promise<int> p;
        folly::Future<int> f = p.getSemiFuture().via(&executor);

        bool flag = false;
        auto f2 = std::move(f).thenValue([&flag](int x) {
            std::cout << "f(" << x << ")" << std::endl;
            flag = true;
            return;
        });

        // Error: ThreadedExecutor.cpp:43 Check failed: !stopping_.load(std::memory_order_acquire)
//        std::move(f2).via(&executor).thenValue([flag](folly::Unit /* not used */) {
//            EXPECT_TRUE(flag);
//        });

        p.setValue(5);
        std::move(f2).get();
    }
}