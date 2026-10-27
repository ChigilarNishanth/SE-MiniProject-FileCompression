# Baseline changes and deviations

Prepared 27 October 2026 against the submitted Part-1 SRS and upstream main dc36070.

| ID | Original statement / implementation | Current treatment | Reason and review impact |
| --- | --- | --- | --- |
| DEV-01 | Project title says Basic ZIP Implementation | Custom .czip Huffman container, explicitly non-PKZIP | The actual submitted scope already specified .czip; title cannot promise ZIP interoperability |
| DEV-02 | Header fields named without a complete byte-order/version contract; scaffold writes native uint16 0x435A | Literal bytes CZ, version/flags, little-endian sizes/frequencies, bit count and CRC-32 | Portable deterministic format and stronger corruption checks; scaffold archives intentionally unsupported |
| DEV-03 | Security objective implies any modification is detectable | Detect accidental corruption; CRC is not authentication | A malicious party can recompute a checksum; cryptographic authenticity needs a separate requirement |
| DEV-04 | Security errors named like return codes, while NFR-05 permits only 0/1/2 | Diagnostic categories SEC_ERR_BAD_HEADER / SEC_ERR_MALFORMED_STREAM, process exit 1 | Resolves inconsistent exit-code language |
| DEV-05 | Design lists several unimplemented .cpp modules | Logical modules consolidated into src/czip.cpp | Small complete implementation; replaced sample-frequency scaffold; component/API mapping updated |
| DEV-06 | Empty-file test permits warning or generic failure | Deterministic success with empty valid archive and zero-size restored file | Clear measurable behavior without data loss |
| DEV-07 | Help test expects stderr | --help to stdout; syntax errors to stderr | Explicit CLI contract; tests updated |
| DEV-08 | Original NFR-01 targets dual-core x86-64 | ARM64 evidence measured locally; x86-64 acceptance left pending | Available host is not the specified reference environment |
| DEV-09 | Original NFR-02 names peak heap | Bounded allocations plus process RSS evidence; formal heap profiling distinguished | RSS is broader than heap and should not be mislabeled |
| DEV-10 | No output conflict or expansion policy stated | No overwrite; staged atomic publication; 1 GiB default cap with override | Protect source files and bound archive expansion; new SEC-REQ-03 |
| DEV-11 | Sprint-1 demo requested during 19-23 October | Current-product demonstration prepared after Sprint 1 | No fabricated historical recording, member-authored PR, or retrospective completion claim |

All changes require team acknowledgement during final review/demo. Original submitted files remain preserved and are not silently replaced.
