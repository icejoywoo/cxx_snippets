#include <gtest/gtest.h>

#include "folly/compression/Compression.h"
#include "folly/io/IOBuf.h"
#include "folly/String.h"

#include "my_lz4.h"

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
};

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
};

INSTANTIATE_TEST_CASE_P(Lz4, CompressionTest, ::testing::Values(
  folly::io::CodecType::LZ4,
  folly::io::CodecType::LZ4_FRAME,
  folly::io::CodecType::LZ4_VARINT_SIZE
));

TEST_F(CompressionTest, MyLz4SimpleTest) {
    const std::string input("abcde_bcdefgh_abcdefghxxxxxxx");
    uint32_t output_length = my::lz4::compressor::maxCompressedLength(input.length());
    auto* output = new uint8_t[output_length];
    memset(output, 0, output_length);
    int actualCompressedLength = my::lz4::compressor::compress(
            (uint8_t*) input.data(), 0, input.length(),
            output, 0, output_length);

    std::string compressed_str = folly::ByteRange(output, actualCompressedLength).str();
    std::string output_str = folly::hexlify(compressed_str);
    EXPECT_STREQ("e161626364655f626364656667685f0e00a066676878787878787878", output_str.c_str());

    uint32_t uncompressed_length = input.length();
    auto* uncompressed_buffer = new uint8_t[uncompressed_length];
    memset(uncompressed_buffer, 0, uncompressed_length);
    int actualUncompressedSize = my::lz4::decompressor::decompress(
            (uint8_t*) compressed_str.data(), 0, compressed_str.length(),
            uncompressed_buffer, 0, uncompressed_length);

    EXPECT_STREQ(input.c_str(), folly::ByteRange(uncompressed_buffer, actualUncompressedSize).str().c_str());
};
namespace {
std::string generateRandomString(int length, time_t seed = 0) {
  std::string result;
  result.reserve(length);
  // 设置随机种子为当前时间 + seed
  std::srand(std::time(0) + seed);

  int loop4 = length / 4;
  for (int i = 0; i < loop4; i++) {
    int tmp = std::rand();
    result.append(reinterpret_cast<char*>(&tmp));
  }
  int left = length % 4;
  for (int i = 0; i < left; i++) {
    result += (char) (std::rand() % 256);
  }

  return result;
}
} // anonymous namespace

TEST_F(CompressionTest, MyLz4AndFollyLz4Compatibility) {
    for (int i = 0; i < 10000; i++) {
        const std::string input = generateRandomString(20 + (i / 10 * 10), i);
        uint32_t output_length = my::lz4::compressor::maxCompressedLength(input.length());
        auto* output = new uint8_t[output_length];
        memset(output, 0, output_length);
        int actualCompressedLength = my::lz4::compressor::compress(
                (uint8_t*) input.data(), 0, input.length(),
                output, 0, output_length);

        std::string compressed_str = folly::ByteRange(output, actualCompressedLength).str();

        auto codec = folly::io::getCodec(folly::io::CodecType::LZ4);
        std::unique_ptr<folly::IOBuf> compressedBuf = folly::IOBuf::copyBuffer(compressed_str);
        std::unique_ptr<folly::IOBuf> uncompressedBuf = codec->uncompress(compressedBuf.get(), input.size());
        std::string uncompressedStr;
        for(auto& b : *uncompressedBuf) {
            uncompressedStr.append(b.begin(), b.end());
        }
        EXPECT_STREQ(input.c_str(), uncompressedStr.c_str());
    }

    for (int i = 0; i < 10000; i++) {
        const std::string input = generateRandomString(20 + (i / 10 * 10), i);
        auto codec = folly::io::getCodec(folly::io::CodecType::LZ4);
        std::unique_ptr<folly::IOBuf> buf = folly::IOBuf::copyBuffer(input);
        std::unique_ptr<folly::IOBuf> compressedBuf = codec->compress(buf.get());
        // write each part to a string
        std::string compressedStr;
        for(auto& b : *compressedBuf) {
            compressedStr.append(b.begin(), b.end());
        }

        uint32_t uncompressed_length = input.length();
        auto* uncompressed_buffer = new uint8_t[uncompressed_length];
        memset(uncompressed_buffer, 0, uncompressed_length);
        int actualUncompressedSize = my::lz4::decompressor::decompress(
                (uint8_t*) compressedStr.data(), 0, compressedStr.length(),
                uncompressed_buffer, 0, uncompressed_length);

        EXPECT_STREQ(input.c_str(), folly::ByteRange(uncompressed_buffer, actualUncompressedSize).str().c_str());
    }
};