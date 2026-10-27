#include "huffman_tree.hpp"
#include <algorithm>
#include <limits>
#include <queue>
#include <stdexcept>

namespace {
struct NodeGreater {
    bool operator()(const HuffmanTree::Node* a, const HuffmanTree::Node* b) const noexcept {
        if (a->frequency != b->frequency) return a->frequency > b->frequency;
        return a->min_symbol > b->min_symbol;
    }
};
}

HuffmanTree::HuffmanTree(const FrequencyTable& frequencies) {
    uint64_t total = 0;
    for (const auto count : frequencies.counts) {
        if (count > std::numeric_limits<uint64_t>::max() - total) {
            throw std::overflow_error("Huffman frequency total exceeds uint64_t");
        }
        total += count;
    }

    std::priority_queue<const Node*, std::vector<const Node*>, NodeGreater> heap;
    auto add_node = [this](uint64_t frequency, unsigned symbol, unsigned minimum,
                          const Node* left, const Node* right) -> const Node* {
        nodes_.push_back(std::make_unique<Node>(Node{frequency, symbol, minimum, left, right}));
        return nodes_.back().get();
    };
    for (unsigned symbol = 0; symbol < 256; ++symbol) {
        if (frequencies.counts[symbol] != 0) {
            heap.push(add_node(frequencies.counts[symbol], symbol, symbol, nullptr, nullptr));
        }
    }
    while (heap.size() > 1) {
        const Node* left = heap.top();
        heap.pop();
        const Node* right = heap.top();
        heap.pop();
        // Every subtree is disjoint, so the validated total bounds each merge.
        heap.push(add_node(left->frequency + right->frequency, 256,
                           std::min(left->min_symbol, right->min_symbol), left, right));
    }
    if (!heap.empty()) {
        root_ = heap.top();
        std::string path;
        generate_codes(root_, path);
    }
}

void HuffmanTree::generate_codes(const Node* node, std::string& path) {
    if (node->is_leaf()) {
        // An explicit bit prevents a one-symbol input from having an empty code.
        codes_[node->symbol] = path.empty() ? "0" : path;
        return;
    }
    path.push_back('0');
    generate_codes(node->left, path);
    path.back() = '1';
    generate_codes(node->right, path);
    path.pop_back();
}
