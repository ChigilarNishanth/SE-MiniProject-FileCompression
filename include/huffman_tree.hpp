#ifndef HUFFMAN_TREE_HPP
#define HUFFMAN_TREE_HPP

#include "common.hpp"
#include <memory>
#include <queue>
#include <vector>
#include <string>

struct HuffmanNode {
    uint8_t ch = 0;
    uint64_t freq = 0;
    std::shared_ptr<HuffmanNode> left = nullptr;
    std::shared_ptr<HuffmanNode> right = nullptr;

    HuffmanNode(uint8_t c, uint64_t f) : ch(c), freq(f) {}
    HuffmanNode(uint64_t f, std::shared_ptr<HuffmanNode> l, std::shared_ptr<HuffmanNode> r)
        : ch(0), freq(f), left(l), right(r) {}
};

struct CompareNode {
    bool operator()(const std::shared_ptr<HuffmanNode>& a, const std::shared_ptr<HuffmanNode>& b) const {
        return a->freq > b->freq;
    }
};

class HuffmanTree {
public:
    static std::shared_ptr<HuffmanNode> build(const FrequencyTable& freq_table);
    static void generate_codes(const std::shared_ptr<HuffmanNode>& root, 
                               std::string current_code, 
                               std::string codes[256]);
};

#endif // HUFFMAN_TREE_HPP