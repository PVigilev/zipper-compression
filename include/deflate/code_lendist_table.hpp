#ifndef DEFLATE_CODELENDIST_TABLE_HPP
#define DEFLATE_CODELENDIST_TABLE_HPP

#include <array>
#include <cstdint>

namespace zipper::deflate
{
    struct code_extrabits_value_entry
    {
        uint16_t code;
        uint8_t  extra_bits;
        uint32_t base_value;
    };
    constexpr std::array<code_extrabits_value_entry, 31> code_lengths_table {
        code_extrabits_value_entry{257,   0,   3},
        code_extrabits_value_entry{258,   0,   4},
        code_extrabits_value_entry{259,   0,   5},
        code_extrabits_value_entry{260,   0,   6},
        code_extrabits_value_entry{261,   0,   7},
        code_extrabits_value_entry{262,   0,   8},
        code_extrabits_value_entry{263,   0,   9},
        code_extrabits_value_entry{264,   0,  10},
        code_extrabits_value_entry{265,   1,  11},
        code_extrabits_value_entry{266,   1,  13},
        code_extrabits_value_entry{267,   1,  15},
        code_extrabits_value_entry{268,   1,  17},
        code_extrabits_value_entry{269,   2,  19},
        code_extrabits_value_entry{270,   2,  23},
        code_extrabits_value_entry{271,   2,  27},
        code_extrabits_value_entry{272,   2,  31},
        code_extrabits_value_entry{273,   3,  35},
        code_extrabits_value_entry{274,   3,  43},
        code_extrabits_value_entry{275,   3,  51},
        code_extrabits_value_entry{276,   3,  59},
        code_extrabits_value_entry{277,   4,  67},
        code_extrabits_value_entry{278,   4,  83},
        code_extrabits_value_entry{279,   4,  99},
        code_extrabits_value_entry{280,   4, 115},
        code_extrabits_value_entry{281,   5, 131},
        code_extrabits_value_entry{282,   5, 163},
        code_extrabits_value_entry{283,   5, 195},
        code_extrabits_value_entry{284,   5, 227},
        code_extrabits_value_entry{285,   0, 258},
        code_extrabits_value_entry{286,   0, 0},
        code_extrabits_value_entry{287,   0, 0}
    };

    constexpr std::array<code_extrabits_value_entry, 32> code_dist_table {
        code_extrabits_value_entry{0,   0,     1},
        code_extrabits_value_entry{1,   0,     2},
        code_extrabits_value_entry{2,   0,     3},
        code_extrabits_value_entry{3,   0,     4},
        code_extrabits_value_entry{4,   1,     5},
        code_extrabits_value_entry{5,   1,     7},
        code_extrabits_value_entry{6,   2,     9},
        code_extrabits_value_entry{7,   2,    13},
        code_extrabits_value_entry{8,   3,    17},
        code_extrabits_value_entry{9,   3,    25},
        code_extrabits_value_entry{10,  4,    33},
        code_extrabits_value_entry{11,  4,    49},
        code_extrabits_value_entry{12,  5,    65},
        code_extrabits_value_entry{13,  5,    97},
        code_extrabits_value_entry{14,  6,   129},
        code_extrabits_value_entry{15,  6,   193},
        code_extrabits_value_entry{16,  7,   257},
        code_extrabits_value_entry{17,  7,   385},
        code_extrabits_value_entry{18,  8,   513},
        code_extrabits_value_entry{19,  8,   769},
        code_extrabits_value_entry{20,  9,  1025},
        code_extrabits_value_entry{21,  9,  1537},
        code_extrabits_value_entry{22, 10,  2049},
        code_extrabits_value_entry{23, 10,  3073},
        code_extrabits_value_entry{24, 11,  4097},
        code_extrabits_value_entry{25, 11,  6145},
        code_extrabits_value_entry{26, 12,  8193},
        code_extrabits_value_entry{27, 12, 12289},
        code_extrabits_value_entry{28, 13, 16385},
        code_extrabits_value_entry{29, 13, 24577},
        code_extrabits_value_entry{30,  0,     0},
        code_extrabits_value_entry{31,  0,     0}
    };

    constexpr std::array<code_extrabits_value_entry, 3> clcl_table {
        code_extrabits_value_entry{16,  2,     3},
        code_extrabits_value_entry{17,  3,     3},
        code_extrabits_value_entry{18,  7,    11},
    };
} // namespace zipper::deflate


#endif