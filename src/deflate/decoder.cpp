#include "bit_buffer.hpp"
#include "deflate/decoder.hpp"
#include "deflate/huffman_tree.hpp"
#include "deflate/huffman_dfa.hpp"


namespace zipper::deflate
{

huffman_tree<LITLEN_CODES> decoder::static_huffman_tree = decoder::build_static_huffman_tree();


huffman_tree<LITLEN_CODES> decoder::build_static_huffman_tree() {
    std::array<uint32_t, LITLEN_CODES> result;
    result.fill(0);

    for(size_t i = 0; i <= 143; i++) {
        result[i] = 8;
    }

    for(size_t i = 144; i <= 255; i++) {
        result[i] = 9;
    }

    for(size_t i = 256; i <= 279; i++) {
        result[i] = 7;
    }

    for(size_t i = 280; i <= 287; i++) {
        result[i] = 8;
    }

    return huffman_tree<LITLEN_CODES>(result);
}


decode_result decoder::decode(uint8_t* target, size_t target_length) {
    size_t target_idx = 0;
    size_t block_number = 0;

    while (target_idx < target_length && !read_buffer.eob()) {
        bool is_last_block = read_buffer.read_bit();
        uint32_t block_type;
        
        if(read_buffer.read_bits(block_type, 2) < 2) {
            return unexpected(decode_failure{read_buffer.byte_offset(), read_buffer.offset(), block_number, "Unexpected end of block"});
        }

        if (block_type == NO_COMPRESSION) {
            auto result = decode_no_compress(target + target_idx, target_length - target_idx);
            if(!result) {
                return result;
            }
            target_idx += result->bytes_written;
        } else if (block_type == STATIC_HUFFMAN) {
            auto result = decode_with_huffman(target + target_idx, target_length - target_idx, static_huffman_tree, 
            
                std::function<decode_result(uint32_t&)>([this](uint32_t& dist_code) {
                    return this->decode_static_huffman_distance(dist_code);
                }));
            if(!result) {
                return result;
            }
            target_idx += result->bytes_written;
        } else if (block_type == DYNAMIC_HUFFMAN) {
            dynamic_block_trees trees;
            auto result = decode_dynamic_huffman_header(trees);

            if(!result) {
                return result;
            }

            result = decode_with_huffman(target + target_idx, target_length - target_idx, trees.litlen_tree, 
                std::function<decode_result(uint32_t&)>([this, &trees](uint32_t& dist_code) {
                    return this->decode_dynamic_huffman_distance_prototype(dist_code, trees.distance_tree);
            }));

            if(!result) {
                return result;
            }
            target_idx += result->bytes_written;
        } else if(block_type == RESERVED) {
            return unexpected(decode_failure{read_buffer.byte_offset(), read_buffer.offset(), block_number, "Compression type is RESERVED"});
        }

        if(is_last_block) {
            break;
        }
        block_number++;
    }

    return decode_success{target_idx, read_buffer.offset()};
}

decode_result decoder::decode_no_compress(uint8_t* target, size_t length) {
    read_buffer.skipt_to_byte();

    if (read_buffer.left_bits() < 4) {
        return unexpected(decode_failure{read_buffer.byte_offset(), read_buffer.offset(), 0, "Unexpected end of input during read length of non-compressed block"});
    }
    auto ptr = read_buffer.data() + read_buffer.byte_offset();

    auto len = *reinterpret_cast<uint16_t*>(ptr);
    ptr += sizeof(len);

    auto nlen = *reinterpret_cast<uint16_t*>(ptr);
    ptr += sizeof(nlen);

    if((len ^ nlen) != 0xFFFF) {
        return unexpected(decode_failure{read_buffer.byte_offset(), read_buffer.offset(), 0, "Corrupted data during read length of non-compressed block"});
    }

    if (length - 2*sizeof(len) < len) {
        return unexpected(decode_failure{read_buffer.byte_offset(), read_buffer.offset(), 0, "Target data is too short"});
    }
    if (read_buffer.left_bits()/8 < len) {
        return unexpected(decode_failure{read_buffer.byte_offset(), read_buffer.offset(), 0, "Source data is too short"});
    }

    for(size_t i = 0; i < len; i++) {
        target[i] = ptr[i];
    }
    read_buffer.skip(len*8);
    return decode_success{len, (len + 2*sizeof(len)) * 8};
}

decode_result decoder::decode_with_huffman(uint8_t* target, size_t length, const huffman_tree<LITLEN_CODES>& tree, std::function<decode_result(uint32_t&)> read_dist_code) {
    decode_success result{0, 0};
    for(size_t target_offset = 0; target_offset < length; target_offset++) {
        size_t init_buff_offset = read_buffer.offset();

        // read code
        huffman_dfa<LITLEN_CODES> dfa(tree);
        uint32_t value = 0;
        auto res = decode_with_dfa(dfa, value);
        if(!res) {
            return res;
        }
        result.bits_read += res->bits_read;


        if(value < 256) { // literal code
            target[target_offset] = value;
            result.bytes_written++;
        } else if (value == 256) { // end of block
            return result;
        } else if (value < LITLEN_CODES) { // length code
            const auto extra_bits = code_lengths_table[value - 256 - 1].extra_bits;
            const auto base_value = code_lengths_table[value - 256 - 1].base_value;

            uint32_t length = 0;
            if(extra_bits != read_buffer.read_bits(length, extra_bits)) {
                return unexpected(decode_failure{read_buffer.byte_offset(), read_buffer.offset(), 0, "Unexpected end of buffer during extra bits read for length"});
            }
            length = base_value + length;


            // read distance code
            uint32_t dist_code = 0;
            auto res = read_dist_code(dist_code);
            if(!res){
                return res;
            }

            const auto dist_extra_bits = code_dist_table[dist_code].extra_bits;
            const auto dist_base_value = code_dist_table[dist_code].base_value;

            uint32_t distance = 0;
            if(dist_extra_bits != read_buffer.read_bits(distance, dist_extra_bits)) {
                return unexpected(decode_failure{read_buffer.byte_offset(), read_buffer.offset(), 0, "Unexpected end of buffer during extra bits read for length"});
            }
            distance = dist_base_value + distance;

            // copy starting from -distance of length `length`
            uint8_t* dist_target = target + target_offset - distance;
            for(size_t i = 0; i < length; i++) {
                target[target_offset + i] = dist_target[i];
            }
            target_offset += length - 1;
            result.bytes_written += length;

        } else {
            return unexpected(decode_failure{read_buffer.byte_offset(), read_buffer.offset(), 0, "Unknown code"});
        }
    }

    return result;
}

decode_result decoder::decode_static_huffman_distance(uint32_t& dist_code){
    dist_code = 0;
    if(read_buffer.read_bits(dist_code, 5) < 5) {
        return unexpected(decode_failure{read_buffer.byte_offset(), read_buffer.offset(), 0, "Unexpected end of input during decoding static distance code"});
    }
    return decode_success{0, 5};
}

decode_result decoder::decode_dynamic_huffman_distance_prototype(uint32_t& dist_code, const huffman_tree<DISTANCE_CODES>& tree) {
    dist_code = 0;
    huffman_dfa<DISTANCE_CODES> dfa(tree);
    size_t i = 0;
    while(dfa.ok() && !dfa.accepted() && !read_buffer.eob()) {
        dfa.consume(read_buffer.read_bit());
        i++;
    }

    if(!dfa.accepted()) {
        return unexpected(decode_failure{read_buffer.byte_offset(), read_buffer.offset(), 0, "Unknown code for distance"});
    }

    dist_code = dfa.value();
    return decode_success{0, i};
}

decode_result decoder::decode_dynamic_huffman_header(dynamic_block_trees& trees) {
    if (read_buffer.left_bits() < 14) {
        unexpected(decode_failure{0,0,0, "Buffer is too small for reading dynamic block header."});
    }
    uint8_t hlit = 0;
    uint8_t hdist = 0;
    uint8_t hclen = 0;

    read_buffer.read_bits(hlit, 5);
    read_buffer.read_bits(hdist, 5);
    read_buffer.read_bits(hclen, 4);

    const uint32_t literal_codes = hlit + 257;
    const uint32_t distance_codes = hdist + 1;
    const uint32_t code_lengths_codes = hclen + 4;

    constexpr static size_t CLEN_CODES = 19;
    constexpr static std::array<uint8_t, CLEN_CODES> clen_order = {
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    huffman_tree<CLEN_CODES> clen_tree;
    std::array<uint32_t, CLEN_CODES> clen_lengths;
    clen_lengths.fill(0);

    // populate clen_lengths
    for(size_t i = 0; i < code_lengths_codes; i++) {
        if(read_buffer.read_bits(clen_lengths[clen_order[i]], 3) != 3) {
            return unexpected(decode_failure{read_buffer.byte_offset(), read_buffer.offset(), 0, "Unexpected end of buffer while reading code lengths code lengths"});
        }
    }

    huffman_tree<CLEN_CODES>::from_lengths(clen_tree, clen_lengths);


    auto r = decode_tree_with_codelens(clen_tree, literal_codes, trees.litlen_tree);
    if(!r) {
        return r;
    }
    r = decode_tree_with_codelens(clen_tree, distance_codes, trees.distance_tree);
    if(!r) {
        return r;
    }


    return decode_success{0, 0};
}
    
} // namespace zipper::deflate
