#include <gtest/gtest.h>

#include "folly/compression/Compression.h"
#include "folly/io/IOBuf.h"
#include "folly/String.h"

// https://stackoverflow.com/a/6256846
class CompressionTest : public ::testing::TestWithParam<folly::io::CodecType> {
  // You can implement all the usual fixture class members here.
  // To access the test parameter, call GetParam() from class
  // TestWithParam<T>.
};

TEST_P(CompressionTest, Lz4) {
    using namespace folly;
    // Call GetParam() here to get the Row values
    io::CodecType const& p = GetParam();
    // LZ4, LZ4_FRAME, LZ4_VARINT_SIZE
    std::unique_ptr<io::Codec> codec = io::getCodec(io::CodecType::LZ4, io::COMPRESSION_LEVEL_FASTEST);
    
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

    {
        std::unique_ptr<IOBuf> uncompressedBuf = codec->uncompress(compressedBuf.get(), input.size());
        std::string out2;
        for(auto& b : *uncompressedBuf) {
            out2.append(b.begin(), b.end());
        }
        std::cout << "uncompressed: " << out2 << std::endl;
        assert(out2 == input);
    }

    {
        // airlift lz4 compressed test
        compressedBuf = IOBuf::copyBuffer(folly::unhexlify("e161626364655f626364656667685f0e00a066676878787878787878"));
        std::unique_ptr<IOBuf> uncompressedBuf = codec->uncompress(compressedBuf.get(), input.size());
        std::string out2;
        for(auto& b : *uncompressedBuf) {
            out2.append(b.begin(), b.end());
        }
        std::cout << "uncompressed: " << out2 << std::endl;
        assert(out2 == input);
    }
}

TEST_F(CompressionTest, Snappy) {
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

INSTANTIATE_TEST_CASE_P(Lz4, CompressionTest, ::testing::Values(
  folly::io::CodecType::LZ4,
  folly::io::CodecType::LZ4_FRAME,
  folly::io::CodecType::LZ4_VARINT_SIZE
));