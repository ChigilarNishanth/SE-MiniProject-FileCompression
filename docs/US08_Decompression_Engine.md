# US-08: Decompression Engine & Safe Bounds Tree Traversal

Sprint 2 story #8. Owner: Mahesh kumar .B (`Mahesh33-mahi`), PES1UG24CS33. Requirement: FR-02. Estimate: 5 story points.

## Description
As a user, I want to execute `czip -d archive.czip restored.txt` to restore original files safely without memory leaks.

## Acceptance Criteria and Implementation

| Criterion | Implementation |
| --- | --- |
| Reconstruct Huffman tree from archive header frequency table | Reads 256-element frequency table from archive header via `ArchiveIO::read_header` and calls `HuffmanTree::build` to deterministically reconstruct the exact Huffman tree. |
| Read bitstream bit-by-bit, traversing tree from root to leaf nodes | Reads packed payload bytes and extracts bits from MSB (bit 7) down to LSB (bit 0); traverses left on bit 0 and right on bit 1 until reaching a leaf node (`!node->left && !node->right`), then outputs `node->ch` and resets traversal to root. |
| Include bounds checking to prevent crashes or infinite loops on malformed bitstreams | Strict bounds checks on `written < original_size`, stream EOF detection (`bytes_read <= 0`), and invalid node traversal (`!node`) prevent crashes, buffer overflows, or infinite loops on truncated or corrupted archives. |

## Memory Safety & Efficiency
- **Zero Memory Leaks**: Tree nodes are managed automatically through `std::shared_ptr<HuffmanNode>`. All temporary heap structures are deallocated upon scope exit, even when error exits are taken.
- **Buffered I/O**: Both stream reading and decoded output writing utilize 64 KB buffers, delivering high decompression throughput (> 100 MB/s).

## Verification

```sh
# 1. Compile and run US-08 Decompression Acceptance Unit Tests
g++ -std=c++17 -Wall -Wextra -Werror -Iinclude \
  src/archive_io.cpp src/huffman_tree.cpp src/bit_stream.cpp \
  src/compression_engine.cpp src/decompression_engine.cpp tests/decompression_tests.cpp \
  -o /tmp/decomp_tests
/tmp/decomp_tests

# 2. Compile full executable
mkdir -p build
g++ -std=c++17 -O2 -Wall -Wextra -Iinclude \
  src/cli_parser.cpp src/archive_io.cpp src/freq_analyzer.cpp \
  src/huffman_tree.cpp src/bit_stream.cpp src/compression_engine.cpp \
  src/decompression_engine.cpp src/main.cpp \
  -o build/czip

# 3. Execute Production Lossless Decoder Verification Suite
python3 tests/test_lossless.py build/czip /tmp/reference_restore --production-decoder
```

All 6 test fixtures (empty, single-symbol, text, all 256 bytes, random binary, and PPM image) achieve 100% SHA-256 and byte-for-byte fidelity with the production CLI decoder.
