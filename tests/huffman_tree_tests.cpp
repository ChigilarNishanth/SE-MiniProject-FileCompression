#include "huffman_tree.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <random>
#include <stdexcept>
#include <vector>

namespace {
void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void inspect(const HuffmanTree::Node* node, const FrequencyTable& frequencies,
             const HuffmanTree::CodeTable& codes, std::string path,
             std::array<bool, 256>& seen, std::size_t& visited) {
    check(node != nullptr, "missing child");
    ++visited;
    if (node->is_leaf()) {
        check(!node->left && !node->right, "leaf has children");
        check(!seen[node->symbol], "duplicate symbol leaf");
        seen[node->symbol] = true;
        check(node->frequency == frequencies.counts[node->symbol], "wrong leaf frequency");
        check(node->min_symbol == node->symbol, "wrong leaf tie-break value");
        check(codes[node->symbol] == (path.empty() ? "0" : path), "code does not match tree path");
    } else {
        check(node->left && node->right, "internal node missing child");
        check(node->frequency == node->left->frequency + node->right->frequency, "wrong parent weight");
        check(node->min_symbol == std::min(node->left->min_symbol, node->right->min_symbol), "wrong subtree minimum");
        inspect(node->left, frequencies, codes, path + '0', seen, visited);
        inspect(node->right, frequencies, codes, path + '1', seen, visited);
    }
}

void validate(const FrequencyTable& frequencies) {
    HuffmanTree tree(frequencies);
    uint64_t total = 0, weighted_cost = 0;
    std::size_t symbols = 0;
    std::priority_queue<uint64_t, std::vector<uint64_t>, std::greater<uint64_t>> weights;
    for (unsigned i = 0; i < 256; ++i) {
        total += frequencies.counts[i];
        if (frequencies.counts[i]) {
            ++symbols;
            weights.push(frequencies.counts[i]);
            check(!tree.codes()[i].empty(), "active byte lacks a code");
            weighted_cost += frequencies.counts[i] * tree.codes()[i].size();
            for (unsigned j = 0; j < 256; ++j) {
                if (i != j && frequencies.counts[j]) {
                    check(tree.codes()[j].compare(0, tree.codes()[i].size(), tree.codes()[i]) != 0,
                          "codes are not prefix-free");
                }
            }
        } else {
            check(tree.codes()[i].empty(), "zero-frequency byte has a code");
        }
    }
    if (!symbols) {
        check(!tree.root() && tree.node_count() == 0, "empty input creates a tree");
        return;
    }
    check(tree.root()->frequency == total, "root frequency differs from total");
    check(tree.node_count() == 2 * symbols - 1, "wrong number of nodes");
    std::array<bool, 256> seen{};
    std::size_t visited = 0;
    inspect(tree.root(), frequencies, tree.codes(), "", seen, visited);
    check(visited == tree.node_count(), "unreachable or shared nodes");
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
    HuffmanTree rebuilt(frequencies);
    check(rebuilt.codes() == tree.codes(), "rebuilding changes the codebook");
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
            FrequencyTable f; f.counts[0] = 100;
            validate(f); HuffmanTree tree(f);
            check(tree.root()->symbol == 0 && tree.codes()[0] == "0", "wrong single-symbol convention");
        });
        run("independent known codebook", [] {
            FrequencyTable f; f.counts['A'] = 3; f.counts['B'] = 2; f.counts['C'] = 1;
            validate(f); HuffmanTree tree(f);
            check(tree.codes()['A'] == "0" && tree.codes()['B'] == "11" && tree.codes()['C'] == "10",
                  "known Huffman codebook differs");
        });
        run("equal-frequency deterministic tie-break", [] {
            FrequencyTable f; for (unsigned i = 'A'; i <= 'D'; ++i) f.counts[i] = 1;
            validate(f); HuffmanTree tree(f);
            check(tree.codes()['A'] == "00" && tree.codes()['B'] == "01" &&
                  tree.codes()['C'] == "10" && tree.codes()['D'] == "11", "unstable tie order");
        });
        run("leaf versus internal-node frequency tie", [] {
            FrequencyTable f; f.counts['A'] = 2; f.counts['B'] = 1; f.counts['C'] = 1;
            validate(f); HuffmanTree tree(f);
            check(tree.codes()['A'] == "0" && tree.codes()['B'] == "10" && tree.codes()['C'] == "11",
                  "internal-node tie does not use subtree minimum");
        });
        run("all 256 byte values", [] {
            FrequencyTable f; for (auto& count : f.counts) count = 1;
            validate(f); HuffmanTree tree(f);
            for (const auto& code : tree.codes()) check(code.size() == 8, "uniform alphabet has non-8-bit code");
            check(tree.codes()[0] == "00000000" && tree.codes()[255] == "11111111", "wrong byte ordering");
        });
        run("ignore zero-frequency entries", [] {
            FrequencyTable f; f.counts[0] = 5; f.counts[255] = 2;
            validate(f); HuffmanTree tree(f);
            check(tree.node_count() == 3, "zero-frequency entries inserted into heap");
        });
        run("frequency overflow rejected and maximum total accepted", [] {
            FrequencyTable f; f.counts[0] = std::numeric_limits<uint64_t>::max(); f.counts[1] = 1;
            bool rejected = false;
            try { HuffmanTree tree(f); } catch (const std::overflow_error&) { rejected = true; }
            check(rejected, "overflow not rejected");
            --f.counts[0];
            HuffmanTree tree(f);
            check(tree.root()->frequency == std::numeric_limits<uint64_t>::max(), "valid maximum total rejected");
        });
        run("deep skewed tree supports long codes", [] {
            FrequencyTable f; f.counts[0] = 1; f.counts[1] = 1;
            for (unsigned i = 2; i < 40; ++i) f.counts[i] = f.counts[i-1] + f.counts[i-2];
            validate(f); HuffmanTree tree(f);
            check(tree.codes()[0].size() > 32, "deep codes were truncated to a machine word");
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
