#ifndef BIT_BUFFER_HPP
#define BIT_BUFFER_HPP
#include <algorithm>
#include <cstdint>
namespace zipper
{

class bit_buffer {
    uint8_t* ptr;
    size_t size_bytes;
    size_t curr_index;
public:
    bit_buffer(uint8_t* p, size_t size_in_bytes, size_t bit_index) : ptr(p), size_bytes(size_in_bytes), curr_index(bit_index) {}
    size_t byte_offset() const { return curr_index/8; }
    bool write_bit(bool value) {
        if(eob()) {
            return false;
        }
        if (value) {
            ptr[curr_index/8] |= 1 << (curr_index % 8);
        } else {
            ptr[curr_index/8] &= ~(1 << (curr_index % 8));
        }
    }

    bool eob() const { return size_bytes * 8 <= curr_index; }

    bool read_bit() {
        bool result = ((ptr[curr_index/8] >> (curr_index % 8)) & 0x1);
        curr_index++;
        return result;
    }

    template<typename T>
    size_t read_bits(T& result, uint32_t length) {
        result = 0;
        if(length == 0) {
            return 0;
        }
        length = size_bytes * 8 - curr_index < length ? size_bytes * 8 - curr_index : length;
        
        for (uint32_t offset = 0; offset < length;) {
            const uint32_t rest_in_byte = 8 - (curr_index % 8); // unread bits in the current byte
            const uint32_t from_cur_byte = (length - offset) < rest_in_byte ? (length - offset) : rest_in_byte; // bits to read from the current byte
            
            result |= ((ptr[curr_index / 8] >> (curr_index % 8)) & ((1 << from_cur_byte) - 1)) << offset;
            offset += from_cur_byte;
            curr_index += from_cur_byte;
        }
        return length;
    }
    
    void skip(size_t bits) { curr_index += bits; }
    void skipt_to_byte() { 
        if(curr_index == 0) {
            return;
        }
        curr_index += (8 - (curr_index % 8));
    }

    size_t offset() const {return curr_index;}

    uint8_t* data() const { return ptr; }

    size_t left_bits() const { return size_bytes * 8 - curr_index;}
};

} // namespace zipper


#endif
