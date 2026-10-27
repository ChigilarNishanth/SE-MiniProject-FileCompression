# Software Test Plan and Test Case Specification

## 1. Test plan identifier and introduction
TP-CZIP-2.0, Team-E17, prepared 27 October 2026. IEEE 829-style test-plan sections cover test items, tested/excluded features, approach, acceptance, suspension, deliverables, tasks, environment, responsibilities, schedule, risks, and approvals. This plan validates the revised SRS against executable behavior. Original course minimum: 7-10 test cases; this baseline supplies 21 automated cases plus two non-functional benchmark cases.

## 2. Test items
Release and sanitizer builds of src/czip.cpp; CLI parser/controller, frequency table, min-heap and codebook, version-1 header, packed bitstream, CRC validation, bounded decoder, transactional output, and statistics. Configuration items: CMakeLists.txt, .github/workflows/ci.yml, tests/test_czip.py, and tools/benchmark.py. Use only generated, non-sensitive fixtures in temporary directories.

## 3. Features to be tested
All FR-01 through FR-08; NFR-01 throughput, NFR-02 memory, NFR-03 SHA-256 losslessness, NFR-04 feedback, NFR-05 exit statuses; SEC-REQ-01 header bounds, SEC-REQ-02 decoder safety, SEC-REQ-03 output preservation and expansion limits. Edge cases include empty input, one symbol, every byte value, high-entropy binary, ties, malformed headers/payload, invalid syntax, output conflicts, and unavailable output directories.

## 4. Features not to be tested
ZIP compatibility, encryption, directories, GUI, networking, parallel coding, and unsupported container versions. Local validation does not establish Linux/Windows compatibility, the original x86-64 performance target, or exhaustive resistance to every malicious input. CRC tests do not prove cryptographic authenticity. Power-loss recovery and hostile local filesystem races are outside the acceptance scope.

## 5. Test approach
Use black-box executable tests so the checks exercise actual file formats, output bytes, exit codes, and publication behavior. Positive tests compare SHA-256 digests or exact bytes; header tests inspect explicit integers; negative tests assert failure, stderr category, absent destination, and cleanup. Deterministically seeded round trips and bit mutations extend fixture coverage without unstable randomness. Repeat the suite under ASan/UBSan. Measure performance separately with an optimized non-sanitized executable, 500,000,000-byte text and binary files, wall-clock time, and SHA-256.

### 5.1 Security validation
Invalid magic/version/flags, truncated headers, impossible frequency sums, oversized declared output, invalid bit counts, truncated/trailing payload, checksum or payload corruption, nonzero padding, invalid single-symbol codes, and 40 seeded one-bit archive mutations must all fail cleanly. The test harness gives each command a 30-second timeout, rejects crashes through expected exit status, and checks for no leaked provisional directory. Existing output and source aliases must retain their bytes. Sanitizer findings fail the run even if ordinary positive output looks correct. These are negative-input tests, not a claim of exhaustive penetration testing.

### 5.2 Requirement-to-test traceability
| Requirement | Automated case(s) / evidence |
| --- | --- |
| FR-01 | TC-01, TC-02, TC-03, TC-04, TC-05, TC-17, TC-18 |
| FR-02 | TC-01 through TC-05, TC-16, TC-17, TC-18 |
| FR-03 | TC-02, TC-06 |
| FR-04 | TC-06 deterministic output; TC-18 varied symbol distributions; TC-21 independent golden archive |
| FR-05 | TC-06, TC-11, TC-12 |
| FR-06 | TC-03, TC-04, TC-06, TC-13, TC-15 |
| FR-07 | TC-07 |
| FR-08 | TC-08, TC-17 |
| NFR-01 | TC-22, benchmark_results.json; x86-64 reference host pending |
| NFR-02 | TC-23, fixed-memory design and process RSS evidence |
| NFR-03 | TC-01 through TC-05, TC-18, TC-22 SHA-256 |
| NFR-04 | TC-09, TC-10, TC-11 through TC-16, TC-20 |
| NFR-05 | TC-08, TC-09, TC-11 through TC-16 |
| SEC-OBJ-01 / SEC-REQ-01 | TC-11, TC-12, TC-16, TC-19 |
| SEC-OBJ-02 / SEC-REQ-02 | TC-11 through TC-15, TC-19; sanitizer log |
| SEC-REQ-03 | TC-10, TC-16, TC-20 |

## 6. Test case specification
Each case starts in a fresh temporary directory with an available compiled executable. The run_cli helper checks the expected exit status; round_trip writes input, compresses it, restores it, and compares SHA-256. Case numbers correspond to test method prefixes.

| Case | Input and procedure | Expected result |
| --- | --- | --- |
| TC-01 | Repeated ASCII text; compress then decompress | Exit 0; restored SHA-256 equals source |
| TC-02 | All 256 byte values repeated 37 times; round trip | Binary-safe byte identity |
| TC-03 | Empty file; round trip | Valid 2,072-byte header-only archive; empty restored file |
| TC-04 | 10,001 bytes of FF hex; round trip | Single-symbol coding decodes exactly |
| TC-05 | Seeded 100,000-byte random input; round trip | Valid archive even if size grows; exact restoration |
| TC-06 | Bytes 0..255 repeated four times; compress twice; inspect header | Identical archives, literal CZ/version/flags, original size 1,024, all 256 counters = 4 |
| TC-07 | 40,000 A bytes; inspect success statistics | Archive smaller than input; original/archive/savings/time labels present |
| TC-08 | Help, missing/unknown flags, invalid/duplicate output caps | Help exit 0 to stdout; syntax errors exit 2 with usage on stderr |
| TC-09 | Compress a nonexistent file | Exit 1; IO_ERROR diagnostic |
| TC-10 | Use source as destination; restore into an existing file | Exit 1; source and existing destination remain unchanged |
| TC-11 | Alter signature/version/flags; truncate header at 0, 1, 12, 2,071 bytes | Exit 1; SEC_ERR_BAD_HEADER; no destination or staging residue |
| TC-12 | Set size, bits, or first frequency to uint64 maximum | Exit 1; reject header/cap/overflow inconsistency |
| TC-13 | Delete final payload byte or append one byte | Exit 1; SEC_ERR_MALFORMED_STREAM; exact-length enforcement |
| TC-14 | Flip checksum byte and packed payload byte independently | Exit 1; integrity failure; no published data |
| TC-15 | Corrupt zero padding or one-symbol code | Exit 1; malformed stream rejection |
| TC-16 | Restore 100-byte input with cap 99, then cap 100 | First rejected without output; second succeeds exactly |
| TC-17 | Paths containing spaces; omit output argument in both modes | Default naming works and restores correct contents |
| TC-18 | 25 seeded varied-length/alphabet fixtures | Every round trip returns identical bytes |
| TC-19 | 40 seeded single-bit mutations across header/payload | Every mutation rejected without crash or leftover files |
| TC-20 | Output under a nonexistent parent directory | Exit 1; no provisional directory remains |
| TC-21 | Independently construct AAABBC header and known payload 1F 00; compare encoder and decode oracle | Exact archive bytes and restored content match independent specification |
| TC-22 | Release build; 500 MB text and binary; wall-clock timing and hash checks | Compression >10 MB/s on specified reference class; identical restored SHA-256. ARM64 result reported separately |
| TC-23 | Record peak process RSS during TC-22 and inspect bounded allocations | Supporting RSS <50,000,000 bytes; no growth proportional to input; heap-specific measurement distinguished |

## 7. Item pass/fail criteria
All 21 automated cases must pass for both Release and sanitizer builds; nonzero unexpected exits, timeout, hash mismatch, clobbered target, published corrupt data, or sanitizer output are failures. Performance acceptance needs the stated reference environment; a fast different host is supporting evidence. Memory acceptance uses bounded-allocation inspection plus RSS evidence, with heap-specific instrumentation recommended for formal sign-off.

## 8. Suspension and resumption
Suspend release review after a data-loss bug, reproducible sanitizer finding, failed hash comparison, or overwritten source. Preserve the failing fixture and log in an issue. Resume after correction, targeted regression, and the full relevant requirement suite. Do not replace missing evidence with a passing declaration.

## 9. Test deliverables
Source tests, Release log test_results_release.txt, sanitizer log test_results.txt, benchmark_results.json, Test_Report.md, and demo command transcripts. Generated fixtures and build binaries stay outside Git. CI logs become separate remote evidence only after execution.

## 10. Tasks and environment
Build Release, run the requirement suite, build with -fsanitize=address,undefined and run the suite, execute 500 MB benchmark, inspect report, render submission PDF, verify diagrams and page layout, and run current-product demonstrations. Tests require Python 3.9+ for randbytes; CI selects Python 3.12. Optional media/PDF tools are authoring dependencies, not application runtime dependencies.

## 11. Responsibilities and schedule
Use the existing upstream story ownership: Nishanth handles CLI/header/integration/tests; Mahesh handles stream analysis/CI/decoder; Mudit handles tree/bit-packing/statistics. These are backlog responsibilities, not assertions that each person authored the generated patch. Review owner must inspect the actual diff and participant-authored PRs.

Original Sprint 1: 19-23 October 2026. Original Sprint 2: 26-30 October 2026. Current evidence was produced after Sprint 1; the missing historical Sprint-1 record cannot be recreated as contemporaneous evidence. Team review, actual PR author/reviewer evidence, and final freeze remain release tasks.

## 12. Risks and contingencies
Hard-link support varies by filesystem; validate the evaluation volume or expect explicit failure. Performance depends on CPU, storage, and compiler; record the host instead of extrapolating. Small and incompressible data can expand; explain header overhead in the demo. The new versioned archive rejects scaffold headers; record this change. Unknown GitHub project permissions can prevent board updates; prepared backlog files do not establish a live board.

## 13. Approvals
Prepared for team and evaluator review. No student signature, instructor acceptance, historical sprint completion, or final release approval is asserted by this generated plan.
