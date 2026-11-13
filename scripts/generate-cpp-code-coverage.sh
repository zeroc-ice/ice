#!/bin/sh

set -e

# Generates a code coverage report for the C++ libraries and executables using llvm-cov.
# The script builds Ice with every language mapping, runs the C++ tests in the current working directory, and writes
# the report to coverage/html.

# The profile file is compiled in for slice2swift, which SwiftPM runs with the environment of its build plugin, and
# exported too, since LLVM_PROFILE_FILE overrides the compiled-in path.
profile_file="${PWD}/coverage/%m.profraw"
export CPPFLAGS="-fprofile-instr-generate=$profile_file -fcoverage-mapping"
export LDFLAGS="-fprofile-instr-generate=$profile_file"
export LLVM_PROFILE_FILE="$profile_file"
export OPTIMIZE=no

case "$(uname)" in
    Darwin)
        ncpu=$(sysctl -n hw.ncpu)
        # SwiftPM's sandbox would keep slice2swift from writing its profile data. The flags otherwise match those of
        # swift/Makefile with OPTIMIZE=no.
        build() { make "-j$ncpu" SWIFT_BUILD_FLAGS="--disable-sandbox --configuration debug" "$@"; }
        # The root Makefile doesn't build MATLAB, so nothing runs slice2matlab.
        unused_compilers="bin/slice2matlab"
        llvm_profdata() { xcrun llvm-profdata "$@"; }
        llvm_cov() { xcrun llvm-cov "$@"; }
        ;;
    Linux)
        ncpu=$(nproc)
        # The instrumentation flags are clang's, and the llvm tools on the PATH must match its version. The Debian
        # rules prefix CXX with the target triple, which clang doesn't install, so the build sets the platform's
        # compiler directly.
        export CXX=clang++
        build() { make "-j$ncpu" "$(dpkg --print-architecture)_cxx=clang++" "$@"; }
        # The root Makefile doesn't build MATLAB, nor Swift on Linux.
        unused_compilers="bin/slice2matlab bin/slice2swift"
        llvm_profdata() { llvm-profdata "$@"; }
        llvm_cov() { llvm-cov "$@"; }
        ;;
    *)
        echo "This script is only supported on macOS and Linux; on Windows, use generate-cpp-code-coverage.ps1"
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
        echo "Generates one code coverage report for the libraries in lib and the executables in bin. The report"
        echo "includes every Slice compiler but slice2matlab, and slice2swift on Linux, which no build runs. It also"
        echo "counts the header code that only the tests use, but not the test code itself."
        echo "The script cleans the whole checkout, builds Ice with code coverage instrumentation, along with every"
        echo "language mapping and its tests, runs the C++ tests, and generates the report in coverage/html. It needs"
        echo "the toolchain of each language mapping that the root Makefile builds."
        echo ""
        echo "example:"
        echo "  cd cpp && ../scripts/generate-cpp-code-coverage.sh"
        echo "  open coverage/html/index.html"
        echo ""
        echo "On Linux, the script supports only Debian and Ubuntu. It builds with clang++ and needs llvm-profdata and"
        echo "llvm-cov of the same version."
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
# distclean, unlike clean, also removes the Slice compilers, so they're rebuilt with instrumentation. The builds of the
# other language mappings and their tests run the other Slice compilers.
make -C .. distclean
build -C ..

echo "Running tests..."
python3 allTests.py --all --workers="$ncpu"

echo "Merging coverage data..."
llvm_profdata merge -o coverage/default.profdata coverage/*.profraw
rm -f coverage/*.profraw

# Each library once, through its versioned file rather than its symlinks, the executables in bin but the Slice
# compilers that nothing ran, and the test executables and plugins. The report leaves out the test code itself, but the
# tests instantiate header templates and inline functions that the libraries don't.
set --
for object in lib/lib*.dylib lib/*/lib*.so* bin/* $(find test -path '*/build/*' -type f); do
    case " $unused_compilers " in
        *" $object "*) continue ;;
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
