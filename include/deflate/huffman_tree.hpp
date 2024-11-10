#ifndef HUFFMAN_TREE_HPP
#define HUFFMAN_TREE_HPP

#include <array>
#include "code.hpp"

namespace zipper::deflate {

template<size_t n_codes>
class huffman_dfa;

template<size_t n_codes>
class huffman_tree {
public:
    static constexpr uint32_t UNDEF = 0xFFFFFFFF;
    struct node {
        uint32_t child_id[2] = {UNDEF, UNDEF};
        uint32_t parent = UNDEF;
    };
private:
    std::array<node, 2 * n_codes - 1> nodes;
    size_t root_id;

    constexpr bool initialize_branch(code c, uint32_t value, uint32_t& free_node) {
        nodes[value] = node{};

        size_t cur_id = root_id;
        for (size_t i = c.length - 1; i >= 1; i--) {
            uint32_t child_type = (c.body >> i) & 0x1;
            uint32_t child_id = nodes[cur_id].child_id[child_type];
            
            if(child_id == UNDEF) {
                if (free_node >= nodes.size()) {
                    return false;
                }
                child_id = free_node;
                nodes[cur_id].child_id[child_type] = free_node;
                nodes[free_node].parent = cur_id;
                free_node++;
            }

            cur_id = child_id;
        }
        nodes[value].parent = cur_id;
        nodes[cur_id].child_id[c.body & 0x1] = value;

        return true;
    }
public:
    huffman_tree() {
        auto undef_node = node();
        nodes.fill(undef_node);
        root_id = UNDEF;
    }

    constexpr huffman_tree(const std::array<uint32_t, n_codes>& lengths) {
        from_lengths(*this, lengths);
    }

    const std::array<node, 2 * n_codes - 1>& get_nodes() const { return nodes; }
    size_t get_root_id() const {return root_id;}

    template<typename T>
    constexpr static bool from_lengths(huffman_tree& tree, const std::array<T, n_codes>& lengths) {
        auto undef_node = node();
        tree.nodes.fill(undef_node);
        tree.root_id = n_codes;

        std::array<uint32_t, sizeof(uint32_t) * 8> code_lengths;
        code_lengths.fill(0);
        

        uint32_t max_bits = 0;
        for(T l: lengths) {
            code_lengths[l]++;
            if(l > max_bits) {
                max_bits = l;
            }
        }

        std::array<uint32_t, sizeof(uint32_t) * 8> next_code;
        next_code.fill(0);
        uint32_t curr_code = 0;
        for (size_t bits = 1; bits <= max_bits; bits++) {
            curr_code = ((curr_code + code_lengths[bits-1]) << 1);
            next_code[bits] = curr_code;
        }

        uint32_t free_node = tree.root_id + 1;
        for (uint32_t i = 0; i < n_codes; i++) {
            const auto len = lengths[i];
            if (len != 0) {
                if(!tree.initialize_branch(code{next_code[len]++, len}, i, free_node)){
                    return false;
                }
            }
        }

        return true;
    }

    friend class huffman_dfa<n_codes>;
};



}


#endif
