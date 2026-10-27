# Test Execution and Benchmark Report

Evidence prepared 27 October 2026. Host: macOS-27.0.1-arm64-arm-64bit.
Compiler: Apple clang version 21.0.0 (clang-2100.1.1.101). Release: -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror.

## Automated validation
21 automated cases passed in Release and ASan/UBSan builds. Cases include 25 seeded round trips, 40 seeded single-bit archive mutations, and an independently constructed golden archive. Detailed logs are test_results_release.txt and test_results.txt. No remote CI result is implied.

## 500 MB non-functional measurements
Each fixture contains 500,000,000 bytes. Timing includes complete CLI process execution and I/O; MB/s uses decimal megabytes. Compression uses two source passes. macOS /usr/bin/time -l reports peak process resident set size in bytes, not an allocator-specific heap measurement.

| Fixture | Compress MB/s | Decompress MB/s | Archive bytes | Peak process RSS (MB) | SHA-256 match |
| --- | --- | --- | --- | --- | --- |
| text | 47.98 | 29.97 | 285,105,010 | 1.70 | True |
| binary | 32.90 | 27.55 | 500,002,072 | 1.79 | True |

## Interpretation and unresolved evidence
Both fixtures exceed 10 MB/s on the available ARM64 host. NFR-01 specifies an x86-64 reference class, so this is supplementary evidence and that exact environment remains unverified. Peak RSS is below 2 MB on this run and supports the bounded-memory design. RSS is broader than heap; no heap-specific profiler output is claimed.

The binary fixture is a deterministic one-megabyte random block repeated 500 times; static per-byte Huffman coding does not exploit long-range block repetition, and the uniform byte distribution makes its payload effectively incompressible. Its 2,072-byte expansion is expected header overhead. Test results are fixture-specific and not a proof for all inputs.

## Reproduction
Build Release, run python3 tests/test_czip.py build/czip, then python3 tools/benchmark.py build/czip 500. For sanitizers, use CMake -DCZIP_SANITIZE=ON or the equivalent compiler flags. Allow roughly 1.5 GB of temporary free disk space for the binary source, archive and restored file.

## Remote CI and publication
Draft PR #13 is published and linked to the existing sprint board as Sprint 2 - In Progress. GitHub Actions run 40556449207 passed all four jobs (Linux, macOS, Windows, Linux sanitizers) for code commit fc2db01268a62ec9518eee7499326f28304f7a17. See remote_validation.json for exact URLs. The initial MSVC warning about mixed bool/integer bit operations was corrected with explicit unsigned conversions. The actions now use verified Node-24 releases checkout v7.0.1 and setup-python v7.1.0.

A follow-up 500 MB run of the corrected CMake Release build also restored both fixtures exactly: 37.62 MB/s text compression and 29.17 MB/s binary compression, with peak RSS below 2 MB. This CMake build uses its Release optimization defaults and ran while demonstration media were being rendered; it is additional correctness/resource evidence, not a controlled comparison against the original -O2 measurements. Raw follow-up data are in benchmark_followup_results.json.
