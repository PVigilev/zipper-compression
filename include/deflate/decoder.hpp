#ifndef DEFLATE_DECODER_HPP
#define DEFLATE_DECODER_HPP
#include <expected>
#include <cstdint>
#include "bit_buffer.hpp"
#include "code_lendist_table.hpp"
#include "decoder_if.hpp"
#include "huffman_tree.hpp"
#include "huffman_dfa.hpp"


#include <functional>

namespace zipper::deflate
{

using std::expected, std::unexpected;

enum compression_type {
    NO_COMPRESSION  = 0b0,
    STATIC_HUFFMAN  = 0b01,
    DYNAMIC_HUFFMAN = 0b10,
    RESERVED        = 0b11
};

constexpr uint32_t CL_CODES = 19;
constexpr uint32_t DISTANCE_CODES = 32;
constexpr uint32_t LITLEN_CODES = 288;

class decoder: decoder_if {
    
    static huffman_tree<LITLEN_CODES> build_static_huffman_tree();

    static huffman_tree<LITLEN_CODES> static_huffman_tree;

    struct dynamic_block_trees {
        huffman_tree<LITLEN_CODES> litlen_tree;
        huffman_tree<DISTANCE_CODES> distance_tree;
    };
    

    bit_buffer read_buffer;

public:
    decoder(uint8_t* source, size_t source_length, size_t start_bit_offset = 0): read_buffer(source, source_length, start_bit_offset) {}

    decode_result decode(uint8_t* target, size_t target_length) override;

    decode_result decode_no_compress(uint8_t* target, size_t length);
    decode_result decode_with_huffman(uint8_t* target, size_t length, const huffman_tree<LITLEN_CODES>& tree, std::function<decode_result(uint32_t&)> dist_decode);
    decode_result decode_static_huffman_distance(uint32_t& dist_code);
    decode_result decode_dynamic_huffman_distance_prototype(uint32_t& dist_code, const huffman_tree<DISTANCE_CODES>& tree);
    decode_result decode_dynamic_huffman_header(dynamic_block_trees& trees);

    template<size_t n_codes>
    decode_result decode_with_dfa(huffman_dfa<n_codes>& dfa, uint32_t& result) {
        dfa.reset();
        size_t n = 0;
        while(dfa.ok() && !dfa.accepted() && !read_buffer.eob()) {
            bool bit = read_buffer.read_bit();
            dfa.consume(bit);
            n++;
        }

        if (!dfa.ok()) {
            return unexpected(decode_failure{read_buffer.byte_offset(), read_buffer.offset(), 0, "Error during decoding using huffman compression: Unknown symbol"});
        }

        if (read_buffer.eob()) {
            return unexpected(decode_failure{read_buffer.byte_offset(), read_buffer.offset(), 0, "Unexpected end of buffer."});
        }

        if (!dfa.accepted()) {
            return unexpected(decode_failure{read_buffer.byte_offset(), read_buffer.offset(), 0, "Incorrect value parsed from buffer"});
        }
        // dfa accepted
        result = dfa.value();

        return decode_success{0, n};
    }
    
    template<size_t n_codes>
    decode_result decode_tree_with_codelens(const huffman_tree<CL_CODES>& tree, uint32_t hcodes, huffman_tree<n_codes>& result) {
        huffman_dfa<CL_CODES> dfa(tree);

        std::array<uint32_t, n_codes> lengths;
        lengths.fill(0);
        size_t n = 0;
        
        for(size_t i = 0; i < hcodes;) {
            uint32_t value = 0;
            auto r = decode_with_dfa(dfa, value);
            if(!r) {
                return r;
            }
            n += r->bits_read;

            // interpret value as a length
            if (value <= 15) {
                lengths[i++] = value;
                continue;
            } else if (value > 18) {
                return unexpected(decode_failure{read_buffer.byte_offset(), read_buffer.offset(), 0, "Unknown value of code length"});
            }

            uint32_t repeats = 0;
            const uint32_t extra_bits = clcl_table[value - 15 - 1].extra_bits,
                           base_value = clcl_table[value - 15 - 1].base_value;
            if (read_buffer.read_bits(repeats, extra_bits) < extra_bits) {
                return unexpected(decode_failure{read_buffer.byte_offset(), read_buffer.offset(), 0, "Unexpected end of buffer"});
            }

            repeats += base_value;
            uint32_t length_to_repeat = value == 16 ? lengths[i - 1] : 0;
            for(size_t j = 0; j < repeats; j++) {
                lengths[i + j] = length_to_repeat;
            }
            i += repeats;
        }

        huffman_tree<n_codes>::from_lengths(result, lengths);
        return decode_success{0, n};
    }
};
} // namespace zipper


#endif
