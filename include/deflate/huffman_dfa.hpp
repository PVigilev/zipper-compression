#ifndef HUFFMAN_DFA_HPP
#define HUFFMAN_DFA_HPP

#include <cstdint>
#include "deflate/huffman_tree.hpp"

namespace zipper::deflate
{

template<size_t n_codes>
class huffman_dfa {
    using Tree = huffman_tree<n_codes>;
    const Tree& tree;
    size_t cur_id;
public:
    huffman_dfa(const Tree& t): tree(t), cur_id{tree.root_id} {}

    bool ok() const {
        return cur_id != Tree::UNDEF;
    }

    bool accepted() const {
        return cur_id < n_codes;
    }

    void consume(bool value) {
        if(!ok()){
            return;
        }
        cur_id = tree.nodes[cur_id].child_id[value];
    }

    void reset() {
        cur_id = tree.root_id;
    }

    uint32_t value() const {
        return accepted() ? cur_id : Tree::UNDEF;
    }
}; 

} // namespace zipper::deflate


#endif
