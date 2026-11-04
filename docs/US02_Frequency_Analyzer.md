# US-02: File Stream I/O & 256-Byte Frequency Analyzer

Sprint 1 story #2. Owner: Mahesh kumar .B (`Mahesh33-mahi`), PES1UG24CS33. Requirement: FR-03. Estimate: 3 story points.

## Description
As a compression engine component, I need to read binary files byte-by-byte and calculate a 256-element frequency distribution array.

## Acceptance Criteria and Implementation

| Criterion | Implementation |
| --- | --- |
| Read input files in binary mode (`rb`) | `std::ifstream` opened with `std::ios::binary` mode to prevent translation of line endings (`\r\n`), EOF markers (`0x1A`), and preserve raw binary values `0x00`-`0xFF`. |
| Calculate frequency counts for all byte values from `0x00` to `0xFF` | 256-element `FrequencyTable.counts` array indexed by `uint8_t` byte values, accumulating counts byte-by-byte. |
| Fail gracefully with error messages if input paths are unreadable | Verifies path validity and unreadable/directory conditions; reports descriptive error messages to `std::cerr` and returns `false` without crashing. |

## API and Conventions

```cpp
#include "freq_analyzer.hpp"

FrequencyTable table;
uint64_t total_bytes = 0;

// Read file in binary mode and compute 256-element frequency distribution
if (!FrequencyAnalyzer::analyze_file("input.bin", table, total_bytes)) {
    // Graceful error handling
}

// Counts for all byte values 0x00 to 0xFF are available:
std::cout << "Count of 0x00: " << table.counts[0x00] << "\n";
std::cout << "Count of 'A':  " << table.counts['A'] << "\n";
```

## Verification

```sh
g++ -std=c++17 -Wall -Wextra -Werror -Iinclude src/freq_analyzer.cpp tests/freq_analyzer_tests.cpp -o /tmp/freq_tests
/tmp/freq_tests
```

Acceptance test cases in `tests/freq_analyzer_tests.cpp` cover:
1. Empty (0-byte) input files yielding all-zero frequency distributions.
2. Full 256-element byte distribution (0x00 through 0xFF) with exact frequency verification.
3. Binary mode (`rb`) preservation across null bytes (`0x00`), CRLF pairs (`\r\n`), Ctrl+Z (`0x1A`), and `0xFF`.
4. Graceful error reporting for non-existent files and directories.
5. In-memory binary stream analysis.
