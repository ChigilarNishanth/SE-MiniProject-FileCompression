#include "huffman_tree.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <array>
#include <queue>
#include <random>
#include <stdexcept>
#include <vector>

namespace {
using CodeTable = std::array<std::string, 256>;

CodeTable make_codes(const std::shared_ptr<HuffmanNode>& root) {
    CodeTable codes{};
    HuffmanTree::generate_codes(root, "", codes.data());
    return codes;
}

void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void inspect(const HuffmanNode* node, const FrequencyTable& frequencies,
             const CodeTable& codes, std::string path,
             std::array<bool, 256>& seen, std::size_t& visited) {
    check(node != nullptr, "missing child");
    ++visited;
    if (!node->left && !node->right) {
        check(!node->left && !node->right, "leaf has children");
        check(!seen[node->ch], "duplicate symbol leaf");
        seen[node->ch] = true;
        check(node->freq == frequencies.counts[node->ch], "wrong leaf frequency");
        check(codes[node->ch] == (path.empty() ? "0" : path), "code does not match tree path");
    } else {
        if (path.empty() && node->left && !node->right) {
            check(node->freq == node->left->freq, "single-symbol parent has wrong weight");
            inspect(node->left.get(), frequencies, codes, "0", seen, visited);
            return;
        }
        check(node->left && node->right, "internal node missing child");
        check(node->freq == node->left->freq + node->right->freq, "wrong parent weight");
        inspect(node->left.get(), frequencies, codes, path + '0', seen, visited);
        inspect(node->right.get(), frequencies, codes, path + '1', seen, visited);
    }
}

void validate(const FrequencyTable& frequencies) {
    const auto root = HuffmanTree::build(frequencies);
    const auto codes = make_codes(root);
    uint64_t total = 0, weighted_cost = 0;
    std::size_t symbols = 0;
    std::priority_queue<uint64_t, std::vector<uint64_t>, std::greater<uint64_t>> weights;
    for (unsigned i = 0; i < 256; ++i) {
        total += frequencies.counts[i];
        if (frequencies.counts[i]) {
            ++symbols;
            weights.push(frequencies.counts[i]);
            check(!codes[i].empty(), "active byte lacks a code");
            weighted_cost += frequencies.counts[i] * codes[i].size();
            for (unsigned j = 0; j < 256; ++j) {
                if (i != j && frequencies.counts[j]) {
                    check(codes[j].compare(0, codes[i].size(), codes[i]) != 0,
                          "codes are not prefix-free");
                }
            }
        } else {
            check(codes[i].empty(), "zero-frequency byte has a code");
        }
    }
    if (!symbols) {
        check(!root, "empty input creates a tree");
        return;
    }
    check(root && root->freq == total, "root frequency differs from total");
    std::array<bool, 256> seen{};
    std::size_t visited = 0;
    inspect(root.get(), frequencies, codes, "", seen, visited);
    check(visited == (symbols == 1 ? 2 : 2 * symbols - 1), "wrong number of nodes");
    for (unsigned i = 0; i < 256; ++i) check(seen[i] == (frequencies.counts[i] != 0), "leaf coverage mismatch");

    // Independent cost oracle: optimal weighted length equals the sum of merge
    // weights, without constructing a tree or generating any codes.
    uint64_t optimal_cost = symbols == 1 ? total : 0;
    while (weights.size() > 1) {
        auto a = weights.top(); weights.pop();
        auto b = weights.top(); weights.pop();
        optimal_cost += a + b;
        weights.push(a + b);
    }
    check(weighted_cost == optimal_cost, "code lengths are not Huffman-optimal");
    check(make_codes(HuffmanTree::build(frequencies)) == codes, "rebuilding changes the codebook");
}
}

int main() {
    unsigned passed = 0;
    auto run = [&passed](const char* name, const std::function<void()>& test) {
        test(); ++passed; std::cout << "PASS: " << name << '\n';
    };
    try {
        run("empty frequency table", [] { validate(FrequencyTable{}); });
        run("single zero-byte symbol", [] {
            FrequencyTable f;
            f.counts[0] = 100;
            validate(f);
            check(make_codes(HuffmanTree::build(f))[0] == "0", "wrong single-symbol convention");
        });
        run("independent known codebook without ties", [] {
            FrequencyTable f;
            f.counts['A'] = 5;
            f.counts['B'] = 2;
            f.counts['C'] = 1;
            validate(f);
            const auto codes = make_codes(HuffmanTree::build(f));
            check(codes['A'] == "1" && codes['B'] == "01" && codes['C'] == "00",
                  "known Huffman codebook differs");
        });
        run("equal-frequency symbols", [] {
            FrequencyTable f;
            for (unsigned i = 'A'; i <= 'D'; ++i) f.counts[i] = 1;
            validate(f);
        });
        run("leaf and internal node with equal weight", [] {
            FrequencyTable f;
            f.counts['A'] = 2;
            f.counts['B'] = 1;
            f.counts['C'] = 1;
            validate(f);
        });
        run("all 256 byte values", [] {
            FrequencyTable f;
            for (auto& count : f.counts) count = 1;
            validate(f);
            for (const auto& code : make_codes(HuffmanTree::build(f))) {
                check(code.size() == 8, "uniform alphabet has non-8-bit code");
            }
        });
        run("ignore zero-frequency entries", [] {
            FrequencyTable f;
            f.counts[0] = 5;
            f.counts[255] = 2;
            validate(f);
        });
        run("deep skewed tree supports long codes", [] {
            FrequencyTable f;
            f.counts[0] = 1;
            f.counts[1] = 1;
            for (unsigned i = 2; i < 40; ++i) {
                f.counts[i] = f.counts[i - 1] + f.counts[i - 2];
            }
            validate(f);
            check(make_codes(HuffmanTree::build(f))[0].size() > 32,
                  "deep codes were truncated to a machine word");
        });
        run("200 seeded distributions: structure, prefix freedom and optimal cost", [] {
            std::mt19937 generator(279);
            for (unsigned trial = 0; trial < 200; ++trial) {
                FrequencyTable f;
                for (auto& count : f.counts) count = generator() % 1001;
                validate(f);
            }
        });
        std::cout << passed << " US-03 tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL after " << passed << " tests: " << error.what() << '\n';
        return 1;
    }
}
