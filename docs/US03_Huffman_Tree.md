# US-03: Min-Heap Priority Queue & Huffman Tree Construction

Sprint 1 story #3. Owner: Mudit Chaturvedi (`InterstellarMC`), PES1UG24CS279. Requirement: FR-04. Existing estimate: 5 points.

## Acceptance criteria and implementation

| Criterion | Implementation |
| --- | --- |
| Insert all nonzero byte frequencies into a min-heap | `std::priority_queue` with frequency/subtree-minimum comparator; zero entries skipped |
| Merge nodes iteratively to form one root | Pop two minimum nodes, use the first as left child, reinsert their summed parent |
| Traverse the tree to generate variable-length bit strings | Left edge appends `0`, right appends `1`; code table indexed by unsigned byte |

The module consumes the team's existing `FrequencyTable` from `common.hpp`. It adds no compression/decompression file processing or CLI changes. Those integrations belong to later stories.

## API and conventions

```cpp
#include "huffman_tree.hpp"
FrequencyTable frequencies;
frequencies.counts['A'] = 3;
frequencies.counts['B'] = 2;
frequencies.counts['C'] = 1;
HuffmanTree tree(frequencies);
const auto& codes = tree.codes();  // A="0", B="11", C="10"
const auto* root = tree.root();   // root->frequency == 6
```

Heap order is ascending `(frequency, minimum byte in subtree)`, including ties between internal nodes and leaves. Both encoder and decoder must use this convention to reconstruct identical codes. An empty table has a null root and empty codes. A single-symbol input receives code `0`. Absent symbols have empty codes. A frequency sum exceeding `uint64_t` throws `std::overflow_error` before constructing nodes. A valid total equal to `uint64_t` maximum is accepted.

The tree owns at most 511 nodes through `unique_ptr`. Root and child pointers are read-only and remain valid only while the tree exists. Copying the owner is disabled. Heap construction costs O(k log k) for k <= 256 symbols; generating explicit strings costs their combined length, with at most 255 edges per code.

## Verification

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -Iinclude \
  src/huffman_tree.cpp tests/huffman_tree_tests.cpp -o /tmp/huffman_tree_tests
/tmp/huffman_tree_tests
```

Ten focused cases cover empty/single-symbol inputs, independent known codes, equal-frequency and leaf/internal ties, all 256 bytes, zero-frequency exclusion, integer overflow, deep codes longer than 32 bits, and 200 seeded frequency distributions. The distributions check tree invariants, prefix freedom, determinism, and optimal weighted code length against an independent merge-cost oracle. The existing GitHub Actions workflow runs these tests alongside its existing CLI/header smoke tests.

Review and merge by the repository owner remain the final acceptance step. The story should be closed when its focused PR is merged.
