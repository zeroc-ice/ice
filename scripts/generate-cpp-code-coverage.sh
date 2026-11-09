#!/bin/sh

set -e

# Generates a code coverage report for the C++ libraries and executables using llvm-cov.
# The script builds and runs the tests in the current working directory, and writes the report to coverage/html.

export CPPFLAGS="-fprofile-instr-generate -fcoverage-mapping"
export LDFLAGS="-fprofile-instr-generate"
export LLVM_PROFILE_FILE="${PWD}/coverage/%m.profraw"
export OPTIMIZE=no

# The llvm tools come from the selected Xcode, so this script is macOS only.
if [ "$(uname)" != "Darwin" ]; then
    echo "This script is only supported on macOS"
    exit 1
fi

usage() {
    echo "usage: $0 [-h|--help]"
}

case "$1" in
    -h|--help)
        usage
        echo ""
        echo "Generates one code coverage report for the libraries in lib and the executables in bin, including the"
        echo "Slice compilers. The report also counts the header code that only the tests use, but not the test code"
        echo "itself."
        echo "The script builds the current working directory with code coverage instrumentation, runs the tests, and"
        echo "generates the report in coverage/html."
        echo ""
        echo "example:"
        echo "  cd cpp && ../scripts/generate-cpp-code-coverage.sh"
        echo "  open coverage/html/index.html"
        exit 0
        ;;
    "")
        ;;
    *)
        usage
        exit 1
        ;;
esac

if [ ! -e "Makefile" ]; then
    echo "Makefile not found"
    exit 1
fi

if [ ! -e "allTests.py" ]; then
    echo "allTests.py not found"
    exit 1
fi

rm -rf coverage

echo "Building with code coverage..."
ncpu=$(sysctl -n hw.ncpu)
# distclean, unlike clean, also removes the Slice compilers, so they're rebuilt with instrumentation.
make distclean
make "-j$ncpu"

echo "Running tests..."
python3 allTests.py --all --workers="$ncpu"

echo "Merging coverage data..."
xcrun llvm-profdata merge -o coverage/default.profdata coverage/*.profraw
rm -f coverage/*.profraw

# Each library once, through its versioned file rather than its symlinks, only the executables in bin, and the test
# executables and plugins. The report leaves out the test code itself, but the tests instantiate header templates and
# inline functions that the libraries don't.
set --
for object in lib/lib*.dylib bin/* $(find test -path '*/build/*' -type f); do
    if [ -f "$object" ] && [ ! -L "$object" ] && [ -x "$object" ]; then
        set -- "$@" -object "$object"
    fi
done

echo "Generating coverage report..."
# Clang instruments the Homebrew headers too, since they aren't system headers.
ignore="^(/opt/homebrew|${PWD}/(test|.*/generated))/"
xcrun llvm-cov show "$@" -instr-profile=coverage/default.profdata -format=html -ignore-filename-regex="$ignore" \
    -output-dir=coverage/html
xcrun llvm-cov report "$@" -instr-profile=coverage/default.profdata -ignore-filename-regex="$ignore" \
    > coverage/report.txt
echo "Coverage report generated in coverage/html/index.html"
