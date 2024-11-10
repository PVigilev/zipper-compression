#include <gtest/gtest.h>
#include <vector>

#include "deflate/huffman_tree.hpp"
#include "deflate/decoder.hpp"

namespace zipper::deflate {

TEST(Huffman, HuffmanTreeBuild) {
    std::array<uint32_t, LITLEN_CODES> lens;
    lens.fill(0);

    for(size_t i = 0; i <= 143; i++) {
        lens[i] = 8;
    }

    for(size_t i = 144; i <= 255; i++) {
        lens[i] = 9;
    }

    for(size_t i = 256; i <= 279; i++) {
        lens[i] = 7;
    }

    for(size_t i = 280; i <= 287; i++) {
        lens[i] = 8;
    }

    huffman_tree<LITLEN_CODES> tree;
    EXPECT_TRUE(huffman_tree<LITLEN_CODES>::from_lengths(tree, lens));
    
    
    std::array<code, LITLEN_CODES> codes;
    for(size_t i = 0; i <= 143; i++) {
        codes[i].body = 0b00110000 + i;
        codes[i].length = 8;
    }

    for(size_t i = 0; i <= 255 - 144 + 1; i++) {
        codes[i + 144].body = 0b110010000 + i;
        codes[i + 144].length = 9;
    }

    for(size_t i = 0; i <= 279 - 256 + 1; i++) {
        codes[i + 256].body = 0b0000000 + i;
        codes[i + 256].length = 7;
    }

    for(size_t i = 0; i <= 287 - 280 + 1; i++) {
        codes[i + 280].body = 0b11000000 + i;
        codes[i + 280].length = 8;
    }

    // verify tree correctness

    const auto nodes = tree.get_nodes();
    for(uint32_t cid = 0; cid < codes.size(); cid++) {
        code c = codes[cid];

        if(c.length == 0) {
            EXPECT_EQ(nodes[cid].parent, huffman_tree<LITLEN_CODES>::UNDEF) << "[" << std::hex << c.body << ", " << std::dec << c.length << "]";
            EXPECT_EQ(nodes[cid].child_id[0], huffman_tree<LITLEN_CODES>::UNDEF) << "[" << std::hex << c.body << ", " << std::dec << c.length << "]";
            EXPECT_EQ(nodes[cid].child_id[1], huffman_tree<LITLEN_CODES>::UNDEF) << "[" << std::hex << c.body << ", " << std::dec << c.length << "]";
            continue;
        }

        uint32_t cur_id = tree.get_root_id();

        for(uint32_t i = c.length - 1; i >= 1; i--) {
            uint32_t child_type = (c.body >> i) & 0x1;
            uint32_t child_id = nodes[cur_id].child_id[child_type];

            EXPECT_NE(child_id, huffman_tree<LITLEN_CODES>::UNDEF) << "[" << std::hex << c.body << ", " << std::dec << c.length << "]" << "(" << i << ")";
            EXPECT_EQ(nodes[child_id].parent, cur_id);
            cur_id = child_id;
        }
        cur_id = nodes[cur_id].child_id[c.body & 0x1];
        EXPECT_EQ(cur_id, cid) << "[" << std::hex << "0x" << c.body << ", " << std::dec << c.length << "]";
        EXPECT_EQ(nodes[cur_id].child_id[0], huffman_tree<LITLEN_CODES>::UNDEF);
        EXPECT_EQ(nodes[cur_id].child_id[1], huffman_tree<LITLEN_CODES>::UNDEF);
    }
}

TEST(DeflateDecoder, NoCompression)
{
    const char *expected = "hello world";
    uint8_t actual[256];

    std::vector<uint8_t> input = {0x0b, 0x00, 0xf4, 0xff, 'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd'};

    decoder d(input.data(), input.size());
    decode_result result = d.decode_no_compress(actual, 256);

    EXPECT_TRUE(result) << result.error().bit_num << " " << result.error().block_number << " " << result.error().byte_offset << " " << result.error().message;
    EXPECT_EQ(result->bytes_written, input.size() - 4);
    
    actual[result->bytes_written] = '\0';
    EXPECT_STREQ(reinterpret_cast<char*>(actual), expected);
}

struct compressed_expected_pair
{
    std::vector<uint8_t> compressed;
    std::vector<uint8_t> expected;
};


class DecoderDeflateFixed : public testing::Test,
    public testing::WithParamInterface<compressed_expected_pair>
{
};

TEST_P(DecoderDeflateFixed, HuffmanBlock)
{
    auto param = GetParam();
    std::vector<uint8_t> actual{};
    actual.resize(param.expected.size());
    deflate::decoder d(
        param.compressed.data(), param.compressed.size());
    
    decode_result result = d.decode(actual.data(), actual.size());

    EXPECT_TRUE(result) << result.error().message;
    EXPECT_EQ(result->bytes_written, param.expected.size());
    EXPECT_EQ(actual, param.expected);
}


INSTANTIATE_TEST_SUITE_P(HuffmanBlock, DecoderDeflateFixed, ::testing::Values(
    compressed_expected_pair
    {
        std::vector<uint8_t>{0xcb, 0x48, 0xcd, 0xc9, 0xc9, 0x57, 0x28, 0xcf, 0x2f, 0xca, 0x49, 0x1, 0x0},
        std::vector<uint8_t>{0x68, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x77, 0x6f, 0x72, 0x6c, 0x64}
    },
    compressed_expected_pair
    {
        std::vector<uint8_t>{0x73, 0x49, 0x4D, 0xCB, 0x49, 0x2C, 0x49, 0x55, 0x00, 0x11, 0x00},
        std::vector<uint8_t>{0x44, 0x65, 0x66, 0x6c, 0x61, 0x74, 0x65, 0x20, 0x6c, 0x61, 0x74, 0x65}
    },
    compressed_expected_pair
    {
        std::vector<uint8_t>{
            0b00001100,	0b11001000,	0b01000001,	0b00001010,	0b10000000,	0b00100000,	0b00010000,	0b00000101,
        	0b11010000,	0b01111101,	0b11010000,	0b00011101,	0b11111110,	0b00001001,	0b10111010,	0b10000100,
            0b11101011,	0b10100000,	0b00101011,	0b01001100,	0b11111010,	0b10110101,	0b00000001,	0b00011101,
            0b00100001,	0b00100111,	0b10100001,	0b11011011,	0b11010111,	0b01011011,	0b10111110,	0b11010000,
            0b10101101,	0b11011100,	0b11100010,	0b01001111,	0b00010101,	0b11010111,	0b01101110,	0b00000011,
            0b11011101,	0b01110000,	0b00110010,	0b11110110,	0b10100110,	0b01010110,	0b00100000,	0b10000110,
            0b00111101,	0b00011100,	0b00011011,	0b10001110,	0b01001010,	0b00011001,	0b11111100,	0b00011111,
            0b10010010,	0b10100110,	0b00001110,	0b00100110,	0b11111000,	0b00100101,	0b00001110,	0b11100110,
            0b11001100,	0b11101000,	0b00111010,	0b00001001,	0b01101101,	0b10001101,	0b01001001,	0b11000101,
            0b01011001,	0b11011111,	0b01110101,	0b11111001,	0b00000110,	0b00000000
            },
        std::vector<uint8_t>{
            0x43, 0x6f, 0x6e, 0x67, 0x72, 0x61, 0x74, 0x75, 0x6c, 0x61, 0x74, 0x69, 0x6f, 
            0x6e, 0x73, 0x20, 0x6f, 0x6e, 0x20, 0x62, 0x65, 0x63, 0x6f, 0x6d, 0x69, 0x6e, 
            0x67, 0x20, 0x61, 0x6e, 0x20, 0x4d, 0x43, 0x50, 0x2e, 0x20, 0x50, 0x6c, 0x65, 
            0x61, 0x73, 0x65, 0x20, 0x62, 0x65, 0x20, 0x61, 0x64, 0x76, 0x69, 0x73, 0x65, 
            0x64, 0x20, 0x74, 0x68, 0x61, 0x74, 0x20, 0x65, 0x66, 0x66, 0x65, 0x63, 0x74, 
            0x69, 0x76, 0x65, 0x20, 0x69, 0x6d, 0x6d, 0x65, 0x64, 0x69, 0x61, 0x74, 0x65, 0x6c, 0x79}
    },
    compressed_expected_pair
    {
        std::vector<uint8_t>{
            0x05, 0xc1, 0xc1, 0x0d, 0xc0, 0x20, 0x0c, 0x03, 0xc0, 0x55, 0x98, 0x2d, 0x38, 0x76, 
            0xd4, 0x47, 0xa5, 0x4, 0xda, 0x88, 0xed, 0xb9, 0xcb, 0xf6, 0xda, 0xe7, 0xe3, 0xb4, 
            0x15, 0x8f, 0x8d, 0x6c, 0x2f, 0x42, 0x84, 0x86, 0xc0, 0x1, 0x2e, 0xa, 0x14, 0x18, 
            0x62, 0xc4, 0xab, 0x6c, 0xaf, 0x7d, 0xf8, 0xdb, 0x34, 0xd0, 0x2e
            },
        std::vector<uint8_t>{
            0x71, 0x77, 0x65, 0x72, 0x74, 0x79, 0x75, 0x66, 0x63, 0x62, 0x73, 0x68, 0x6a, 0x62,
            0x20, 0x71, 0x77, 0x65, 0x72, 0x66, 0x64, 0x67, 0x66, 0x64, 0x67, 0x20, 0x67, 0x64,
            0x66, 0x20, 0x64, 0x66, 0x73, 0x66, 0x67, 0x64, 0x66, 0x67, 0x64, 0x66, 0x68, 0x67,
            0x66, 0x68, 0x68, 0x6e, 0x67, 0x71, 0x77, 0x65, 0x72, 0x74, 0x79, 0x66, 0x76, 0x62,
            0x63, 0x62, 0x64, 0x66, 0x62
        }
    }
));

}