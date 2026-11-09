#!/bin/sh

set -e

# Generates a code coverage report for the C++ libraries and executables using llvm-cov.
# The script builds and runs the tests in the current working directory, and writes the report to coverage/html.

export CPPFLAGS="-fprofile-instr-generate -fcoverage-mapping"
export LDFLAGS="-fprofile-instr-generate"
export LLVM_PROFILE_FILE="${PWD}/coverage/%m.profraw"
export OPTIMIZE=no

case "$(uname)" in
    Darwin)
        ncpu=$(sysctl -n hw.ncpu)
        build() { make "-j$ncpu"; }
        llvm_profdata() { xcrun llvm-profdata "$@"; }
        llvm_cov() { xcrun llvm-cov "$@"; }
        ;;
    Linux)
        ncpu=$(nproc)
        # The instrumentation flags are clang's, and the llvm tools on the PATH must match its version. The Debian
        # rules prefix CXX with the target triple, which clang doesn't install, so the build sets the platform's
        # compiler directly.
        export CXX=clang++
        build() { make "-j$ncpu" "$(dpkg --print-architecture)_cxx=clang++"; }
        llvm_profdata() { llvm-profdata "$@"; }
        llvm_cov() { llvm-cov "$@"; }
        ;;
    *)
        echo "This script is only supported on macOS and Linux"
        exit 1
        ;;
esac

usage() {
    echo "usage: $0 [-h|--help]"
}

case "$1" in
    -h|--help)
        usage
        echo ""
        echo "Generates one code coverage report for the libraries in lib and the executables in bin. Of the Slice"
        echo "compilers, the report includes only slice2cpp: the others are exercised by the builds and tests of their"
        echo "own language mappings, which the script doesn't run. The report also counts the header code that only"
        echo "the tests use, but not the test code itself."
        echo "The script builds the current working directory with code coverage instrumentation, runs the tests, and"
        echo "generates the report in coverage/html."
        echo ""
        echo "example:"
        echo "  cd cpp && ../scripts/generate-cpp-code-coverage.sh"
        echo "  open coverage/html/index.html"
        echo ""
        echo "On Linux, the script builds with clang++ and needs llvm-profdata and llvm-cov of the same version."
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
# distclean, unlike clean, also removes the Slice compilers, so they're rebuilt with instrumentation.
make distclean
build

echo "Running tests..."
python3 allTests.py --all --workers="$ncpu"

echo "Merging coverage data..."
llvm_profdata merge -o coverage/default.profdata coverage/*.profraw
rm -f coverage/*.profraw

# Each library once, through its versioned file rather than its symlinks, only the executables in bin, and the test
# executables and plugins. The report leaves out the test code itself, but the tests instantiate header templates and
# inline functions that the libraries don't.
set --
for object in lib/lib*.dylib lib/*/lib*.so* bin/* $(find test -path '*/build/*' -type f); do
    case "$object" in
        # The other Slice compilers are exercised by the builds and tests of their own language mappings.
        bin/slice2cpp) ;;
        bin/slice2*) continue ;;
    esac
    if [ -f "$object" ] && [ ! -L "$object" ] && [ -x "$object" ]; then
        set -- "$@" -object "$object"
    fi
done

echo "Generating coverage report..."
# Clang instruments the Homebrew headers too, since they aren't system headers.
ignore="^(/opt/homebrew|${PWD}/(test|.*/generated))/"
llvm_cov show "$@" -instr-profile=coverage/default.profdata -format=html -ignore-filename-regex="$ignore" \
    -output-dir=coverage/html
llvm_cov report "$@" -instr-profile=coverage/default.profdata -ignore-filename-regex="$ignore" \
    > coverage/report.txt
echo "Coverage report generated in coverage/html/index.html"
