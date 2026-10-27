# czip - File Compression Tool

Software Engineering Mini-Project, **Team-E17**. A working C++17 command-line compressor using deterministic Huffman coding and a custom `.czip` container. This format is independent of PKZIP and cannot be opened with `unzip`.

| Member | SRN | GitHub |
| --- | --- | --- |
| Mahesh Kumar B | PES1UG24CS255 | Mahesh33-mahi |
| Nishanth T Chigilar | PES1UG24CS302 | ChigilarNishanth |
| Mudit Chaturvedi | PES1UG24CS279 | InterstellarMC |

## Build and run

Requires a C++17 compiler; CMake 3.16+ and Python 3.9+ support the portable test build.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure

# macOS / Linux (Windows multi-config: build/Release/czip.exe)
./build/czip -c input.txt archive.czip
./build/czip -d archive.czip restored.txt
./build/czip --help
```

Direct compiler alternative:

```sh
mkdir -p build
c++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Werror src/czip.cpp -o build/czip
python3 tests/test_czip.py build/czip
```

Output is optional: compression appends `.czip`; decompression removes a final `.czip` or appends `.out`. Existing output files are never overwritten. Quote paths with spaces. Both commands print byte counts, savings, and elapsed time. Small and high-entropy files can grow because each archive has a 2,072-byte header. Exit codes: `0` success, `1` execution/integrity failure, `2` invalid syntax.

The default decoded-output cap is 1 GiB. Set a deliberate limit for larger trusted archives:

```sh
./build/czip -d large.czip restored.bin --max-output 2147483648
```

Archives contain literal `CZ` bytes, version/flags, explicit little-endian metadata, CRC-32, byte frequencies and packed Huffman payload. The decoder validates the complete stream before publishing output. CRC detects accidental corruption; it does not authenticate maliciously rewritten archives. Atomic output publication requires hard-link support on the destination filesystem.

## Verification

21 CLI cases cover round trips, all byte values, empty/single-symbol files, deterministic encoding, an independent golden archive, syntax, corruption, limits, and preservation of existing files. The suite also includes 25 seeded round trips and 40 seeded archive mutations. Local Release and ASan/UBSan logs are under `docs/`. [GitHub Actions run 40556449207](https://github.com/ChigilarNishanth/SE-MiniProject-FileCompression/actions/runs/40556449207) passed macOS, Linux and Windows builds/tests plus Linux sanitizers for code commit `fc2db01`.

```sh
cmake -S . -B build/sanitize -DCZIP_SANITIZE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build/sanitize
ctest --test-dir build/sanitize --output-on-failure
python3 tools/benchmark.py build/czip 500
```

Measured locally on ARM64 macOS with 500 MB fixtures: about **48 MB/s text** and **33 MB/s binary** compression, peak process RSS below 2 MB, and matching SHA-256 hashes. The original x86-64 performance reference and formal team release review remain pending. See [test report](docs/Test_Report.md) for exact measurements and limits.

## Submission materials

- [Combined submission PDF](output/pdf/SE_Mini_Project_Submission.pdf)
- [Revised SRS](docs/SRS_v2.md), [test plan and 23 cases](docs/Test_Plan.md), [architecture/design](docs/Architecture_Design.md)
- [Backlog and two-sprint plan](docs/Backlog_Sprints.md), [machine-readable backlog](docs/backlog.json)
- [Test evidence](docs/Test_Report.md), [deviations](docs/Deviations.md), [release checklist](docs/Submission_Checklist.md)
- [Current-product demos and transcripts](demos/README.md)
- [Original submitted baseline](docs/SRS_Deliverable1.md) and [original PDF](docs/SRS_Deliverable1.pdf)

The implementation branch incorporates upstream `dc36070` and replaces its sample-frequency/header-only scaffold with complete compression/decompression. Original team history remains intact. This package does not claim a historical Sprint-1 recording, PRs authored by other teammates, reviewer approval, or a completed release freeze. Existing upstream issues are referenced rather than duplicated in the fork.

[Draft PR #13](https://github.com/ChigilarNishanth/SE-MiniProject-FileCompression/pull/13) contains the published review candidate and is linked to the existing sprint board as Sprint 2 - In Progress. Owner acceptance remains pending; extra delivery tasks are proposed in the package rather than silently changing existing issue states.

## Rebuild documents and media

Authoring dependencies are separate from the application: ReportLab and Pillow for the PDF, ImageIO-FFmpeg for the terminal replay videos. PyMuPDF can render the PDF for inspection. Install these into your preferred authoring environment, then run:

```sh
python3 tools/prepare_delivery.py
python3 tools/make_demos.py build/czip
python3 tools/build_submission.py
```

`prepare_delivery.py` reads public upstream issue metadata and local benchmark evidence; it performs no GitHub writes. Demo videos replay actual captured commands, with timing adapted for presentation. They are current-product demonstrations, not recordings from an earlier sprint week.
