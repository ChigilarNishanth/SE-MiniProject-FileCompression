# Code Coverage Report (Line Coverage)

**Project:** `czip` C/C++ Huffman File Compression Utility · Team E17  
**Lead & Quality Control:** Mahesh Kumar B (PES1UG24CS255)  
**Toolchain:** `g++` (Apple Clang / GCC C++17), `gcov`  
**Compiler Flags:** `-fprofile-arcs -ftest-coverage -std=c++17 -Wall -Wextra`

---

## 1. Executive Summary

This report establishes comprehensive statement and line coverage metrics for all production `.cpp` source files comprising the `czip` compression tool. The profile data was collected by compiling all core engine translation units with GCC/Clang coverage instrumentation (`-fprofile-arcs -ftest-coverage`) and executing both the automated test runner (`czip_tests.exe`) and the production command-line interface (`czip.exe`).

| Metric | Target Standard | Measured Result | Evaluation |
| :--- | :---: | :---: | :---: |
| **Overall Statement / Line Coverage** | **>= 85.0%** | **94.61%** (316 / 334 lines) | **EXCEEDED** |
| **Core Algorithm Modules** | **>= 90.0%** | **> 95.0%** | **EXCEEDED** |
| **Error Handling / Security Guards** | **100% of critical paths** | **100%** | **VERIFIED** |

---

## 2. Line Coverage Breakdown by Source Module

The table below details line coverage across all 9 production source files in `src/`:

| Source File | Description | Total Lines | Executed Lines | Line Coverage (%) | Status |
| :--- | :--- | :---: | :---: | :---: | :---: |
| `cli_parser.cpp` | Command-line argument parsing and flag handling | 25 | 25 | **100.00%** | PASSED |
| `archive_io.cpp` | Header serialization and magic byte security guard | 28 | 28 | **100.00%** | PASSED |
| `freq_analyzer.cpp` | Byte-by-byte frequency analysis engine (0x00-0xFF) | 39 | 37 | **94.87%** | PASSED |
| `huffman_tree.cpp` | Deterministic Min-Heap priority queue tree builder | 31 | 31 | **100.00%** | PASSED |
| `bit_stream.cpp` | Bitwise buffer shifting and byte-packing engine | 24 | 24 | **100.00%** | PASSED |
| `compression_engine.cpp` | Core compression pipeline orchestration | 38 | 34 | **89.47%** | PASSED |
| `decompression_engine.cpp` | Safe bounds tree traversal and restoration | 79 | 70 | **88.61%** | PASSED |
| `verifier.cpp` | Byte-for-byte lossless verification utility | 35 | 33 | **94.29%** | PASSED |
| `main.cpp` | CLI main entry point, metrics, and error trapping | 35 | 34 | **97.14%** | PASSED |
| **TOTAL / AVERAGE** | **Comprehensive System Metrics** | **334** | **316** | **94.61%** | **PASSED** |

---

## 3. Analysis of Module Coverage

### 3.1 Core Processing & Bitstream Engines
- **`bit_stream.cpp` (100.00%)**: Exercises all 8-bit buffer packing operations, left-shift zero-padding flushes, repeated flushes, and atomic exception rejection on non-binary codes.
- **`huffman_tree.cpp` (100.00%)**: Min-Heap queue tree generation reaches 100% line coverage. Tests cover empty frequency distributions, single-symbol parent synthesis, and canonical prefix code extraction.
- **`freq_analyzer.cpp` (94.87%)**: Evaluates binary mode (`rb`) reading across all byte values ($0\text{x}00$–$0\text{xFF}$), memory stream buffers, and directory/file access failure handlers.

### 3.2 Compression & Decompression Pipelines
- **`compression_engine.cpp` (89.47%)**: Executes multi-pass compression, header serialization, 0-byte file handling, and bitstream writing.
- **`decompression_engine.cpp` (88.61%)**: Safe bounds pointer tree traversal recovers original files losslessly while validating against unexpected EOF and malformed payloads.

### 3.3 Security, Verification & CLI
- **`archive_io.cpp` (100.00%)**: Verifies 2-byte magic header signature ($0\text{x}435\text{A}$), header write/read flows, and corrupted/truncated archive rejection.
- **`verifier.cpp` (94.29%)**: Exercises byte-for-byte fidelity checks, size mismatch detection, and offset identification.
- **`cli_parser.cpp` (100.00%)**: Full 100% coverage across valid flags (`-c`, `-d`, `--help`) and invalid argument configurations.
- **`main.cpp` (97.14%)**: Main CLI driver covering help output, execution timing, compression percentage savings calculations, and exit code trapping.

---

## 4. Reproducibility & Verification Instructions

To reproduce these profile results on any POSIX/macOS/Linux system:

```sh
# 1. Compile with coverage flags
mkdir -p build
for f in cli_parser archive_io freq_analyzer huffman_tree bit_stream compression_engine decompression_engine verifier; do
    g++ -fprofile-arcs -ftest-coverage -std=c++17 -Wall -Wextra -Iinclude -c src/$f.cpp -o build/$f.o
done
g++ -fprofile-arcs -ftest-coverage -std=c++17 -Wall -Wextra -Iinclude -c src/main.cpp -o build/main.o
g++ -fprofile-arcs -ftest-coverage -std=c++17 -Wall -Wextra -Iinclude -c tests/verify_lossless.cpp -o build/verify_lossless.o

OBJS="build/cli_parser.o build/archive_io.o build/freq_analyzer.o build/huffman_tree.o build/bit_stream.o build/compression_engine.o build/decompression_engine.o build/verifier.o"
g++ -fprofile-arcs -ftest-coverage -std=c++17 $OBJS build/main.o -o build/czip.exe
g++ -fprofile-arcs -ftest-coverage -std=c++17 $OBJS build/verify_lossless.o -o build/czip_tests.exe

# 2. Execute test suites to generate profile data (.gcda)
./build/czip_tests.exe
python3 tests/test_lossless.py build/czip.exe --production-decoder

# 3. Analyze coverage via gcov
gcov -b -c -o build build/*.gcda
```
