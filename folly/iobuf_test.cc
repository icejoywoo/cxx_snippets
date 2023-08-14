#include "folly/io/IOBuf.h"

#include <gtest/gtest.h>

TEST(IOBufTest, Basic) {
    std::unique_ptr<folly::IOBuf> buf(new folly::IOBuf(folly::IOBuf::CREATE, 1024));
    // 向IOBuf对象中写入数据
    void* data = buf->writableData();
    memcpy(data, "Hello, world!", 13);
    buf->append(13);

    // 释放IOBuf对象
    buf.reset();
    buf = nullptr;
}