"""Build and run the isolated C++ unit suites in a temporary directory."""

import argparse
import os
from pathlib import Path
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parent.parent
SUITES = (
    ("huffman_tree_tests", "huffman_tree", False),
    ("bit_stream_tests", "bit_stream", True),
    ("freq_analyzer_tests", "freq_analyzer", True),
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cxx", default=os.environ.get("CXX", "g++"),
                        help="path to a GCC or Clang C++ compiler")
    args = parser.parse_args()

    with tempfile.TemporaryDirectory(prefix="czip-unit-") as directory:
        work = Path(directory)
        for suite, module, needs_fixture in SUITES:
            executable = work / (suite + (".exe" if os.name == "nt" else ""))
            print(f"Running {suite}", flush=True)
            subprocess.run(
                [args.cxx, "-std=c++17", "-Wall", "-Wextra", "-I", str(ROOT / "include"),
                 str(ROOT / "src" / f"{module}.cpp"), str(ROOT / "tests" / f"{suite}.cpp"),
                 "-o", str(executable)],
                check=True, cwd=work, timeout=120,
            )
            command = [str(executable)]
            if needs_fixture:
                command.append(str(work / f"{suite}.bin"))
            subprocess.run(command, check=True, cwd=work, timeout=30)


if __name__ == "__main__":
    main()
