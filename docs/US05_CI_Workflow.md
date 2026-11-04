# US-05: Automated C/C++ CI Build & Testing Workflow

Sprint 1 story #5. Owner: Mahesh kumar .B (`Mahesh33-mahi`), PES1UG24CS33. Estimate: 2 story points.

## Description
As a team developer, I want GitHub Actions to compile and test C/C++ code automatically on every Pull Request.

## Acceptance Criteria and Implementation

| Criterion | Implementation |
| --- | --- |
| Workflow triggers on `push` and `pull_request` to `main` | Configured triggers for `push: branches: [main]` and `pull_request: branches: [main]` in `.github/workflows/ci.yml`. |
| Compiles source code using `g++ -std=c++17 -Wall -Wextra` | Executable compilation uses standard C++17 flags `-std=c++17 -Wall -Wextra` with include path `-Iinclude`. |
| Executes unit tests and fails the build if compilation fails | GitHub Actions steps compile and execute all unit test suites (`tests/freq_analyzer_tests.cpp`, `tests/bit_stream_tests.cpp`, `tests/test_lossless.py`); any non-zero exit code on compilation or execution fails the build immediately. |

## CI Workflow Pipeline Structure

```yaml
name: C++ CI

on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]

jobs:
  build-and-test:
    runs-on: ubuntu-latest
    steps:
      - Checkout repository (actions/checkout@v4)
      - Setup build tools (build-essential, cmake, g++)
      - Compile czip utility (g++ -std=c++17 -O2 -Wall -Wextra -Iinclude ...)
      - Verify executable (--help)
      - Compile and run Frequency Analyzer unit tests (tests/freq_analyzer_tests.cpp)
      - Compile and run Bitstream unit tests (tests/bit_stream_tests.cpp)
      - Compile and run SHA-256 lossless acceptance suite (tests/test_lossless.py)
```

## Failure Modes & Safeguards
- **Compilation Failure**: Any syntax error, missing header, or unresolved symbol terminates the pipeline with exit code 1.
- **Test Assertion Failure**: Any failed assertion in the unit tests halts subsequent steps and marks the PR check as failed.
- **Least-Privilege Permissions**: Workflow operates with `contents: read` to maintain security.
