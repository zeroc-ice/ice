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
        echo "Generates one code coverage report for the libraries in lib and the executables in bin. Of the Slice"
        echo "compilers, the report includes only slice2cpp, the one the C++ build and tests run."
        echo "The script builds the current working directory with code coverage instrumentation, runs the tests, and"
        echo "generates the report in coverage/html. A test failure doesn't stop the report: the script generates it"
        echo "and then exits with the test status."
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
test_status=0
python3 allTests.py --all --workers="$ncpu" || test_status=$?

echo "Merging coverage data..."
xcrun llvm-profdata merge -o coverage/default.profdata coverage/*.profraw
rm -f coverage/*.profraw

# Each library once, through its versioned file rather than its symlinks, and only the executables in bin.
set --
for object in lib/lib*.dylib bin/*; do
    case "$object" in
        # slice2cpp is the only Slice compiler the C++ build and tests run.
        bin/slice2cpp) ;;
        bin/slice2*) continue ;;
    esac
    if [ -f "$object" ] && [ ! -L "$object" ] && [ -x "$object" ]; then
        set -- "$@" -object "$object"
    fi
done

echo "Generating coverage report..."
ignore='.*/(generated|test)/.*'
xcrun llvm-cov show "$@" -instr-profile=coverage/default.profdata -format=html -ignore-filename-regex="$ignore" \
    -output-dir=coverage/html
xcrun llvm-cov report "$@" -instr-profile=coverage/default.profdata -ignore-filename-regex="$ignore" \
    > coverage/report.txt
echo "Coverage report generated in coverage/html/index.html"

exit "$test_status"
