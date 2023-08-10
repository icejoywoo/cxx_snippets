#include <gtest/gtest.h>

#include "folly/compression/Compression.h"
#include "folly/io/IOBuf.h"
#include "folly/String.h"

TEST(CompressionTest, Lz4) {
    using namespace folly;
    // LZ4, LZ4_FRAME, LZ4_VARINT_SIZE
    std::unique_ptr<io::Codec> codec = io::getCodec(io::CodecType::LZ4);
    
    const std::string input("abcde_bcdefgh_abcdefghxxxxxxx");
    std::unique_ptr<IOBuf> buf = IOBuf::copyBuffer(input);
    std::unique_ptr<IOBuf> compressedBuf = codec->compress(buf.get());
    // write each part to a string
    std::string out;
    for(auto& b : *compressedBuf) {
        out.append(b.begin(), b.end());
    }
    std::cout << "compressed: " << hexlify(out) << std::endl;
    std::cout << "compressed: " << out << std::endl;

    std::unique_ptr<IOBuf> uncompressedBuf = codec->uncompress(compressedBuf.get(), input.size());
    std::string out2;
    for(auto& b : *uncompressedBuf) {
        out2.append(b.begin(), b.end());
    }
    std::cout << "uncompressed: " << out2 << std::endl;
    assert(out2 == input);
}

TEST(CompressionTest, Snappy) {
    using namespace folly;
    // LZ4, LZ4_FRAME, LZ4_VARINT_SIZE
    std::unique_ptr<io::Codec> codec = io::getCodec(io::CodecType::SNAPPY);
    
    const std::string input("abcde_bcdefgh_abcdefghxxxxxxx");
    std::unique_ptr<IOBuf> buf = IOBuf::copyBuffer(input);
    std::unique_ptr<IOBuf> compressedBuf = codec->compress(buf.get());
    // write each part to a string
    std::string out;
    for(auto& b : *compressedBuf) {
        out.append(b.begin(), b.end());
    }
    std::cout << "compressed: " << hexlify(out) << std::endl;
    std::cout << "compressed: " << out << std::endl;

    std::unique_ptr<IOBuf> uncompressedBuf = codec->uncompress(compressedBuf.get(), input.size());
    std::string out2;
    for(auto& b : *uncompressedBuf) {
        out2.append(b.begin(), b.end());
    }
    std::cout << "uncompressed: " << out2 << std::endl;
    assert(out2 == input);
}