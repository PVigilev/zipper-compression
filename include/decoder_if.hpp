#ifndef DECODER_HPP
#define DECODER_HPP

#include <expected>
#include <stddef.h>

namespace zipper {

struct decode_failure {
    size_t byte_offset;
    size_t bit_num;
    size_t block_number;
    const char* message;
};

struct decode_success {
    size_t bytes_written;
    size_t bits_read;
};

using std::expected;
using decode_result = expected<decode_success, decode_failure>;
class decoder_if {
public:
    virtual decode_result decode(uint8_t* target, size_t target_length) = 0;
};

}

#endif