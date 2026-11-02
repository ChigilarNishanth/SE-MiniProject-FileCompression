#include "huffman_tree.hpp"

std::shared_ptr<HuffmanNode> HuffmanTree::build(const FrequencyTable& freq_table) {
    std::priority_queue<std::shared_ptr<HuffmanNode>, 
                        std::vector<std::shared_ptr<HuffmanNode>>, 
                        CompareNode> pq;

    for (int i = 0; i < 256; ++i) {
        if (freq_table.counts[i] > 0) {
            pq.push(std::make_shared<HuffmanNode>(static_cast<uint8_t>(i), freq_table.counts[i]));
        }
    }

    if (pq.empty()) {
        return nullptr;
    }

    if (pq.size() == 1) {
        auto single = pq.top();
        pq.pop();
        auto parent = std::make_shared<HuffmanNode>(single->freq, single, nullptr);
        return parent;
    }

    while (pq.size() > 1) {
        auto left = pq.top(); pq.pop();
        auto right = pq.top(); pq.pop();

        auto parent = std::make_shared<HuffmanNode>(left->freq + right->freq, left, right);
        pq.push(parent);
    }

    return pq.top();
}

void HuffmanTree::generate_codes(const std::shared_ptr<HuffmanNode>& root, 
                                 std::string current_code, 
                                 std::string codes[256]) {
    if (!root) return;

    if (!root->left && !root->right) {
        codes[root->ch] = current_code.empty() ? "0" : current_code;
        return;
    }

    generate_codes(root->left, current_code + "0", codes);
    generate_codes(root->right, current_code + "1", codes);
}