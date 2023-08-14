#include <gtest/gtest.h>

#include <memory>

struct Foo { // object to manage
    Foo() { 
        std::cout << "Foo...\n";
    }
    ~Foo() { 
        std::cout << "~Foo...\n";
    }
};
 
TEST(PtrTest, UniquePtr) {
    // unique_ptr 在 reset 或者赋值为 nullptr 的时候，会释放持有的对象
    {
        std::unique_ptr<Foo> foo = std::make_unique<Foo>();
        std::unique_ptr<Foo> bar = std::move(foo);
        foo = nullptr;
        bar = nullptr;
    }

    {
        std::unique_ptr<Foo> foo = std::make_unique<Foo>();
        foo.reset();
    }
}