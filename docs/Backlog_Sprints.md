# Backlog and Two-Sprint Plan

Prepared 27 October 2026. Existing upstream stories US-01 through US-10 and their assignees are preserved. Story points for US-01 through US-10 preserve the existing board Estimate values. US-01 through US-05 retain their observed Sprint-1 allocation; US-06 through US-10 are currently in Backlog and are proposed for Sprint 2. US-11 through US-15 are proposed additions. The existing linked board is SE Mini-Project Sprint Board, project #2 under ChigilarNishanth.

Existing project: https://github.com/users/ChigilarNishanth/projects/2 (private; observed in the authenticated browser). No new Project is needed.

Target repository: https://github.com/ChigilarNishanth/SE-MiniProject-FileCompression

Sprint 1: 19-23 October 2026; Sprint 2: 26-30 October 2026. Sprint 1 is already past; this plan does not backdate work.

| Story | Points | Sprint | Owner | Requirements | Evidence |
| --- | --- | --- | --- | --- | --- |
| US-01 CLI Argument Parser & Help Navigation | 2 | 1 | ChigilarNishanth | FR-08, NFR-04, NFR-05 | TC-08, TC-17 |
| US-02 File Stream I/O & 256-Byte Frequency Analyzer | 3 | 1 | Mahesh33-mahi | FR-03, NFR-02 | TC-02, TC-06, TC-22, TC-23 |
| US-03 Min-Heap Priority Queue & Huffman Tree Construction | 5 | 1 | InterstellarMC | FR-04 | TC-06, TC-18, TC-21 |
| US-04 Archive Header Serialization & Magic Byte Validation (0x435A) | 3 | 1 | ChigilarNishanth | FR-05, SEC-REQ-01 | TC-06, TC-11, TC-12 |
| US-05 Automated C/C++ CI Build & Testing Workflow | 2 | 1 | Mahesh33-mahi | NFR-03, NFR-05 | CI matrix |
| US-06 Bitstream Buffer Packing & Bit-Shift File Writer | 5 | 2 | InterstellarMC | FR-06 | TC-04, TC-15, TC-21 |
| US-07 End-to-End File Compression Engine Integration | 5 | 2 | ChigilarNishanth | FR-01 | TC-01 through TC-07, TC-18 |
| US-08 Decompression Engine & Safe Bounds Tree Traversal | 5 | 2 | Mahesh33-mahi | FR-02, SEC-REQ-02 | TC-01 through TC-05, TC-11 through TC-19 |
| US-09 CLI Benchmark Output & Performance Statistics Display | 2 | 2 | InterstellarMC | FR-07, NFR-01 | TC-07, TC-22 |
| US-10 SHA-256 Lossless Verification & Automated Test Suite | 3 | 2 | ChigilarNishanth | NFR-03, SEC-REQ-02 | TC-01 through TC-21 |
| US-11 Streaming performance and memory acceptance | 3 | 2 | InterstellarMC | NFR-01, NFR-02 | TC-22, TC-23; benchmark_results.json |
| US-12 Output preservation and decompression resource limits | 3 | 2 | Mahesh33-mahi | SEC-REQ-03, NFR-04 | TC-10, TC-16, TC-20 |
| US-13 Complete SRS, test plan, design and change log | 3 | 2 | ChigilarNishanth | FR-01 through FR-08, NFR-01 through NFR-05 | docs and submission PDF |
| US-14 Demonstration evidence and contributor review | 3 | 2 | InterstellarMC | Part-2 demonstration and review process | demos; PR and review evidence pending |
| US-15 Final review and development freeze | 2 | 2 | ChigilarNishanth | Part-2 final documentation and freeze | docs/Submission_Checklist.md |

## Sprint 1 goal and capacity
Planned total: 15 story points. Goal: CLI, frequency model, tree, header validation and CI, consistent with the observed board allocation.

## Sprint 2 goal and capacity
Planned total: 34 story points. Goal: packed encoding, complete compression and safe decompression, measured non-functional behavior, documentation, current demonstration, and release review.

## Definition of ready and done
Ready: linked requirement, owner, points, measurable acceptance, dependencies, and fixture plan. Done: implementation reviewed, relevant tests pass, remote CI verified, documents updated, demonstration evidence linked, and actual assignee/reviewer participation recorded. Local implementation alone does not close upstream issues.

## GitHub publication procedure
Reuse existing issues #1 through #10 in the team repository; do not duplicate them in the fork. Reuse the existing linked Project #2 and its Estimate field. Retain current issue states; add requirement/evidence links and proposed Sprint-2 items after review. Keep status based on actual acceptance, not the old sprint dates. The fork has issues disabled, so it is the contribution source, not the team backlog.

## Acceptance details

### US-01
Help, syntax, default output names, and status codes verified.
Ownership: existing upstream issue. GitHub state observed: closed. Current local state: implemented; team review pending.

### US-02
All byte values counted correctly; bounded buffers and RSS evidence.
Ownership: existing upstream issue. GitHub state observed: open. Current local state: implemented; team review pending.

### US-03
Stable frequency/minimum-symbol tie-break and independent golden archive.
Ownership: existing upstream issue. GitHub state observed: open. Current local state: implemented; team review pending.

### US-04
Versioned portable header; invalid metadata rejected before allocation.
Ownership: existing upstream issue. GitHub state observed: closed. Current local state: implemented; team review pending.

### US-05
CMake builds/tests on push and PR; remote jobs must actually run.
Ownership: existing upstream issue. GitHub state observed: open. Current local state: remote Linux/macOS/Windows and sanitizer jobs passed; owner review pending.

### US-06
Known-code payload, one-symbol coding, and zero padding verified.
Ownership: existing upstream issue. GitHub state observed: open. Current local state: implemented; team review pending.

### US-07
Complete stable two-pass compression, not a header-only scaffold.
Ownership: existing upstream issue. GitHub state observed: open. Current local state: implemented; team review pending.

### US-08
Safe decoding with frequency, size, length, padding and CRC validation.
Ownership: existing upstream issue. GitHub state observed: open. Current local state: implemented; team review pending.

### US-09
Actual original/archive/savings/time metrics; benchmark evidence attached.
Ownership: existing upstream issue. GitHub state observed: open. Current local state: implemented; team review pending.

### US-10
21 requirement tests, seeded cases, independent golden oracle and sanitizers.
Ownership: existing upstream issue. GitHub state observed: open. Current local state: implemented; team review pending.

### US-11
Record 500 MB text/binary speed and RSS; validate x86-64 reference target separately.
Ownership: proposed; team confirmation pending. GitHub state observed: not created. Current local state: prepared; acceptance/review pending.

### US-12
Never overwrite an existing file; cleanup handled failures; reject output beyond cap.
Ownership: proposed; team confirmation pending. GitHub state observed: not created. Current local state: prepared; acceptance/review pending.

### US-13
Include UML use case, component view, two sequence diagrams, APIs, security, cases and traceability.
Ownership: proposed; team confirmation pending. GitHub state observed: not created. Current local state: prepared; acceptance/review pending.

### US-14
Current working-product demos: 60 and 120 seconds. Actual member-authored PR and owner review remain human evidence.
Ownership: proposed; team confirmation pending. GitHub state observed: not created. Current local state: prepared; acceptance/review pending.

### US-15
Confirm deviations, CI, unresolved reference-host check, real review, then freeze an approved commit. No backdated closure.
Ownership: proposed; team confirmation pending. GitHub state observed: not created. Current local state: prepared; acceptance/review pending.
