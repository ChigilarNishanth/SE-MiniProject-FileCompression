#ifndef HUFFMAN_TREE_HPP
#define HUFFMAN_TREE_HPP

#include "common.hpp"
#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

// US-03 / FR-04: deterministic model shared by future encoder and decoder.
// This class consumes the existing FrequencyTable; it performs no file I/O.
class HuffmanTree {
public:
    struct Node {
        uint64_t frequency;
        unsigned symbol;      // 0..255 for leaves; 256 for internal nodes.
        unsigned min_symbol;  // Smallest byte in this subtree; stable tie-break.
        const Node* left;
        const Node* right;

        bool is_leaf() const noexcept { return symbol < 256; }
    };
    using CodeTable = std::array<std::string, 256>;

    // Throws std::overflow_error if the sum of frequencies exceeds uint64_t.
    explicit HuffmanTree(const FrequencyTable& frequencies);
    HuffmanTree(const HuffmanTree&) = delete;
    HuffmanTree& operator=(const HuffmanTree&) = delete;

    const Node* root() const noexcept { return root_; }
    const CodeTable& codes() const noexcept { return codes_; }
    std::size_t node_count() const noexcept { return nodes_.size(); }

private:
    // Own every node once; child/root pointers remain valid for this lifetime.
    std::vector<std::unique_ptr<Node>> nodes_;
    const Node* root_ = nullptr;
    CodeTable codes_{};

    void generate_codes(const Node* node, std::string& path);
};

#endif
