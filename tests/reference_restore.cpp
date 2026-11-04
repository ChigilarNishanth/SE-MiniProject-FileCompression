// Test-only restoration oracle. This is NOT the production US-08 decoder.
#include "archive_io.hpp"
#include "huffman_tree.hpp"
#include <fstream>
#include <stdexcept>
int main(int argc, char** argv) {
    if (argc != 3) return 2;
    std::ifstream input(argv[1], std::ios::binary);
    FrequencyTable frequencies; uint64_t size = 0;
    if (!ArchiveIO::read_header(input, size, frequencies) || size > 10000000) return 1;
    auto root = HuffmanTree::build(frequencies);
    auto node = root;
    std::ofstream output(argv[2], std::ios::binary);
    uint64_t written = 0;
    while (written < size) {
        const int byte = input.get(); if (byte < 0) return 1;
        for (int bit = 7; bit >= 0 && written < size; --bit) {
            if (!node) return 1;
            node = (byte & (1 << bit)) ? node->right : node->left;
            if (!node) return 1;
            if (!node->left && !node->right) {
                output.put(static_cast<char>(node->ch)); ++written; node = root;
            }
        }
    }
    return output ? 0 : 1;
}
