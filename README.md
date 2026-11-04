# `czip` - C/C++ Huffman File Compression Utility

**Course:** Software Engineering Mini-Project

**Team ID:** Team-E17

**Repository Owner & Administrator:** Nishanth T Chigilar ([@ChigilarNishanth](https://github.com/ChigilarNishanth))

## Team Details & Division of Work

### Team Roster

| Member Name | SRN | Role & Leadership Focus | GitHub Tag | 
| ----- | ----- | ----- | ----- | 
| **Nishanth T Chigilar** | **PES1UG24CS302** | **Project Lead & System Architect** *(Repo Creator, Admin, Sprint Manager)* | [@ChigilarNishanth](https://github.com/ChigilarNishanth) | 
| **Mahesh Kumar B** | **PES1UG24CS255** | **Backend & Quality Control Engineer** | [@MaheshKumarB](https://github.com/MaheshKumarB) | 
| **Mudit Chaturvedi** | **PES1UG24CS279** | **Core Algorithm & Stream Engineer** | [@MuditChaturvedi](https://github.com/MuditChaturvedi) | 

### Comprehensive Work Division Matrix

#### Nishanth T Chigilar (Team Lead & Lead Developer)

* **Project Leadership & Repository Administration**:

  * Created, configured, and administered the central GitHub repository, branch protection rules, and issue trackers.

  * Formulated the project roadmap, managed Sprint 1 & Sprint 2 backlog items, and led team coordination.

* **Core Systems & Architectural Implementation**:

  * **`US-01` CLI Parser**: Built robust command-line argument parser handling flags (`-c`, `-d`, `--help`), validating file paths, and trapping process exit codes (`0`, `1`, `2`).

  * **`US-04` Header Serialization & Security Guard**: Implemented $0\text{x}435\text{A}$ magic signature (`'CZ'`) validation (`SEC-REQ-01`) and original size payload serialization.

  * **`US-07` Compression Engine Orchestration**: Designed the central pipeline tying together frequency analysis, tree generation, and bitstream output.

  * **`US-10` Lossless Verification Suite**: Created automated verification harness (`LosslessVerifier::compare_files`) for byte-for-byte fidelity checks.

* **Documentation & Validation Deliverables**:

  * Authored **Software Requirements Specification** (`docs/SRS_Deliverable1.md` / `.pdf`).

  * Authored **Software Architecture & Design Specification** (`docs/SAD.md`).

  * Authored **Design & Implementation Deviations Log** (`docs/Deviations.md`).

  * Authored **Updated Test Execution Report** (`docs/Test_Execution_Report.md` - TC-01 to TC-10).

  * Authored **System Validation & Integrity Assurance Report** (`docs/System_Validation_Report.md`).

  * Recorded and published **Sprint 1 & Sprint 2 Video Demonstrations** (`docs/Sprint1_Demo.mp4`, `docs/Sprint2_Demo.mp4`).

#### Mahesh Kumar B (Backend & Quality Control Engineer)

* **Module Implementations**:

  * **`US-02` Frequency Analyzer**: Implemented binary file stream scanner populating 256-element byte frequency array (`docs/US02_Frequency_Analyzer.md`).

  * **`US-05` CI/CD Pipeline Workflow**: Configured GitHub Actions automated build and test runner (`.github/workflows/ci.yml`, `docs/US05_CI_Workflow.md`).

  * **`US-08` Decompression Engine**: Developed Huffman tree reconstruction from header frequencies and safe bounds-checked pointer traversal (`docs/US08_Decompression_Engine.md`).

* **Part-3 Testing & Quality Assurance**:

  * **`Code_Coverage_Report.md`**: Configured `gcov`/`lcov` toolchain for statement line coverage metrics.

  * **`Branch_Coverage_Report.md`**: Analyzed conditional branch execution paths and decision coverage.

#### Mudit Chaturvedi (Core Algorithm & Stream Engineer)

* **Module Implementations**:

  * **`US-03` Huffman Tree Builder**: Built deterministic Min-Heap priority queue (`std::priority_queue`) and canonical code extraction (`docs/US03_Huffman_Tree.md`).

  * **`US-06` Bitstream Serialization**: Developed `BitWriter` and `BitReader` bitwise shift packing engines for 8-bit byte buffers.

  * **`US-09` Performance Analytics**: Built execution timing (ms) and compression ratio savings CLI reporter.

* **Part-3 Testing & Quality Assurance**:

  * **`Unit_Test_Report.md`**: Implemented isolated unit test cases for core modules and boundary edge cases (empty files, single-byte files, uniform character sets).

## System Architecture & Software Design

`czip` is architected using the **Pipe-and-Filter Pattern**. Data streams through modular, decoupled components that perform isolated transformations:

```
+--------------------+       Raw Byte Stream      +--------------------+      Frequency Table     +--------------------+
|  Host File System  | -------------------------> | Frequency Analyzer | -----------------------> | Huffman Tree Build |
|    (Input File)    |                            |      (US-02)       |                          |      (US-03)       |
+--------------------+                            +--------------------+                          +--------------------+
                                                                                                                |
                                                                                                                | Canonical
                                                                                                                | Bitcodes
                                                                                                                v
+--------------------+     Restored Byte Stream   +--------------------+     Packed Bitstream     +--------------------+
|  Host File System  | <------------------------- | Decompress Engine  | <----------------------- | Header & Bitstream |
|   (Restored File)  |                            | (US-08 Tree Trav)  |                          |   (US-04 / US-06)   |
+--------------------+                            +--------------------+                          +--------------------+

```

## Build & Compilation Instructions

### Prerequisites

* **Compiler**: `g++` with C++17 support (MinGW-w64 on Windows, GCC/Clang on Linux/macOS).

* **Build System**: Terminal/PowerShell or Make.

### Build Commands

```
# Create build directory
mkdir build

# Compile czip Main Binary
g++ -std=c++17 -Wall -Wextra -Iinclude src/cli_parser.cpp src/archive_io.cpp src/huffman_tree.cpp src/bit_stream.cpp src/compression_engine.cpp src/decompression_engine.cpp src/verifier.cpp src/main.cpp -o build/czip.exe

# Compile Test Suite Binary
g++ -std=c++17 -Wall -Wextra -Iinclude src/cli_parser.cpp src/archive_io.cpp src/huffman_tree.cpp src/bit_stream.cpp src/compression_engine.cpp src/decompression_engine.cpp src/verifier.cpp tests/verify_lossless.cpp -o build/czip_tests.exe

```

## Usage & CLI Examples

### 1. Compress File

To compress any text or binary file into a `.czip` archive:

```
.\build\czip.exe -c source.txt archive.czip

```

### 2. Decompress File

To extract a `.czip` archive back to its exact byte representation:

```
.\build\czip.exe -d archive.czip restored.txt

```

### 3. Display Usage Manual

```
.\build\czip.exe --help

```

### 4. Run Automated Integration Tests

```
.\build\czip_tests.exe

```

## Security Architecture & Exit Codes

### Header Security Guard (`0x435A`)

Every `.czip` archive begins with the magic header signature `0x435A` (`'CZ'`). During extraction, `ArchiveIO` validates this signature before heap allocation. Invalid or corrupted archives trigger immediate process termination with Exit Code `101`.

### Exit Code Mapping

| Exit Code | Protocol Constant | Trigger Condition | 
| ----- | ----- | ----- | 
| **`0`** | `EXIT_SUCCESS` | Normal completion of compression, decompression, or test suite. | 
| **`1`** | `EXIT_FAILURE` | File access failure, missing stream handle, or permissions issue. | 
| **`2`** | `EXIT_INVALID_CLI` | Unrecognized flags or missing positional CLI parameters. | 
| **`101`** | `SEC_ERR_BAD_HEADER` | Invalid header signature (`!= 0x435A`). | 
| **`102`** | `SEC_ERR_MALFORMED_STREAM` | Unexpected EOF or invalid bitstream tree pointer navigation. | 

## Repository Documentation Index (`docs/`)

### Core Deliverables

* 📄 [Software Requirements Specification (SRS Markdown)](docs/SRS_Deliverable1.md)

* 📄 [Software Requirements Specification (SRS PDF)](docs/SRS_Deliverable1.pdf)

* 📄 [Software Architecture & Design (SAD) Specification](docs/SAD.md)

* 📄 [Design & Implementation Deviations Log](docs/Deviations.md)

### Verification & Testing Reports

* 📄 [Updated IEEE Test Execution Report (TC-01 to TC-10)](docs/Test_Execution_Report.md)

* 📄 [System Validation & Integrity Assurance Report](docs/System_Validation_Report.md)

* 📄 [Unit Test Report](docs/Unit_Test_Report.md)

* 📄 [Code Coverage Report](docs/Code_Coverage_Report.md)

* 📄 [Branch Coverage Report](docs/Branch_Coverage_Report.md)

### User Story Technical Docs & Video Demos

* 🎥 [Sprint 1 Video Demonstration MP4](docs/Sprint1_Demo.mp4)

* 🎥 [Sprint 2 Video Demonstration MP4](docs/Sprint2_Demo.mp4)

* 📝 [US-02 Frequency Analyzer Technical Specs](docs/US02_Frequency_Analyzer.md)

* 📝 [US-03 Huffman Tree Technical Specs](docs/US03_Huffman_Tree.md)

* 📝 [US-05 GitHub Actions CI Workflow Technical Specs](docs/US05_CI_Workflow.md)

* 📝 [US-08 Decompression Engine Technical Specs](docs/US08_Decompression_Engine.md)

* 📝 [Mudit Sprint 2 Contribution Log](docs/Mudit_Sprint2.md)

## License & Compliance

Developed for the **Software Engineering Mini-Project** course evaluation. Built purely in $C_{++}17$ without external compression libraries (e.g., `zlib`), adhering strictly to course compliance rules.