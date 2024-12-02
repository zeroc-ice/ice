#!/bin/sh

set -e

# Generates code coverage report for the specified binary or library using llvm-cov.
# The script runs `make` and `allTests.py` to generate the coverage data in the current working directory.

export CPPFLAGS="-fprofile-instr-generate -fcoverage-mapping"
export LDFLAGS="-fprofile-instr-generate"
export LLVM_PROFILE_FILE="${PWD}/coverage/%m.profraw"
export OPTIMIZE=no

# Use llvm from homebrew if we're on macOS
if [ "$(uname)" != "Darwin" ]; then
    echo "This script is only supported on macOS"
    exit 1
fi

usage() {
    echo "usage: $0 [<path-to-binary-or-library>] [-h|--help]"
}

if [ ! -e "Makefile" ]; then
    echo "Makefile not found"
    exit 1
fi

if [ ! -e "allTests.py" ]; then
    echo "allTests.py not found"
    exit 1
fi

arg=$1
case "$arg" in
    -h|--help)
        usage
        echo ""
        echo "Generates a code coverage report for the specified binary or library. The report is generated in the coverage/html directory."
        echo "The script builds the current working directory with code coverage instrumentation, runs the tests, and generates the report."
        echo "The llvm package must be installed."
        echo ""
        echo "example: generate code coverage report for libIce"
        echo "  cd cpp && ../scripts/generate-code-coverage.sh lib/libIce.dylib"
        echo "  open coverage/html/libIce.dylib/index.html"
        exit 0
        ;;
esac

CLI_TOOLS=/Library/Developer/CommandLineTools/usr/bin

if [ -z "$CLI_TOOLS" ]; then
    echo "Xcode Command Line Tools not found. Run 'xcode-select --install' to install the tools."
    exit 1
fi

if [ -e default.profdata ]; then
    echo "Skipping build and test as profdata already exists..."
    echo "Remove default.profdata to rebuild code coverage"
else
    echo "Building with code coverage..."
    ncpu=$(sysctl -n hw.ncpu)
    make clean
    make "-j$ncpu"

    echo "Running tests..."
    python3 allTests.py --all --workers="$ncpu"

    echo "Merge coverage data..."
    ${CLI_TOOLS}/llvm-profdata merge -o default.profdata coverage/*.profraw

    echo "Cleaning up..."
    rm -f coverage/*.profraw
fi

if [ -n "$arg" ]; then
    echo "Generating coverage for $arg"
    ${CLI_TOOLS}/llvm-cov show "$arg" -instr-profile=default.profdata -format=html -o "./coverage/html/$(basename "$arg")"
    echo "Coverage report generated in coverage/html/$(basename "$arg")/index.html"
    exit 0
else
    echo "Run again with an argument to generate coverage for a specific binary or library"
    exit 0
fi
