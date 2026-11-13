# Part 3 — Unit Test Report

**Project:** czip Huffman File Compression Utility · Team E17  
**Contributor:** Mudit Chaturvedi · PES1UG24CS279 · @InterstellarMC

## Test scope

The Part 3 unit tests exercise the Huffman tree, bit writer and frequency analyzer as separate C++ executables. Each suite links only the module it exercises. A separate CLI run checks the empty, one-byte and uniform-file cases through compression and decompression, and verifies the reported compression statistics.

## Results

| Suite | Cases | Checks | Result |
| --- | ---: | --- | --- |
| Huffman tree (US-03) | 9 | Empty table, single symbol, known codebook, equal-weight nodes, all 256 symbols, unused symbols, deep codes, and 200 seeded frequency distributions | Pass |
| Bit writer (US-06) | 6 | Exact packed bytes for 0, 1, 7, 8 and 9 bits, plus repeated flush, destructor flush and rejection of invalid bits and codes | Pass |
| Frequency analyzer (US-02) | 5 | Empty file, uniform distribution of all 256 bytes, binary control bytes, missing file and directory errors, and in-memory stream counting | Pass |
| Production CLI (US-09 / US-10) | 7 | Empty file, one NUL byte, repeated symbol, text, all byte values, seeded random data and a PPM image | Pass |

For the Huffman suite, every active symbol must appear once, every generated code must match its tree path and no active code may be a prefix of another. The suite also compares weighted code lengths with the sum of minimum-weight merges. Equal-weight cases check repeatability within the current build. They do not establish cross-platform archive compatibility for tied weights.

The writer tests compare the actual bytes with fixed expected values, including `FE` for seven one bits and `B2 80` for the nine bits `101100101`. The invalid-code case checks that the valid prefix of an invalid string does not enter the buffer. For the production CLI, all seven restored files matched both the original bytes and SHA-256 digest. The suite checked reported sizes against the filesystem and computed savings independently.

## Reproduce

From the repository root with Python 3.9+ and GCC or Clang with C++17 support:

```sh
python3 tests/run_unit_tests.py
mkdir -p build
c++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp -o build/czip
python3 tests/test_lossless.py build/czip --production-decoder
```

Verified locally on macOS with Apple Clang. The unit runner uses a temporary directory for binaries and fixtures and exits nonzero on compilation or test failure. CI runs these checks before the existing integration suite.
