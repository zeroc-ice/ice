# OSS Scanner build of Ice (https://github.com/zeroc-ice/ice).
#
# The scanner builds this file with the Ice checkout as the build context. The network is available only while this
# file builds; the scan itself runs offline, so every toolchain and package dependency is fetched here.
#
# Two trees are built from the same checkout:
#
#   /src            The checkout. C++ (sources and tests), Python, PHP, Java, C# and JavaScript, built with
#                   OPTIMIZE=no: debug info, no optimization, and asserts enabled. Run tests from here, for example
#                   `python3 cpp/allTests.py --filter=Ice/stream`, or `python3 allTests.py --filter=...` in another
#                   language directory.
#   /opt/ice-asan   A copy of the checkout with only C++ (sources and tests), built with AddressSanitizer and
#                   UndefinedBehaviorSanitizer. Use it to confirm memory-safety findings:
#                   `python3 /opt/ice-asan/cpp/allTests.py --filter=Ice/stream`.
#
# Swift (Apple platforms only) and MATLAB (proprietary) are not built. Ruby is not built either: Ice for Ruby
# requires Ruby 3.4, and Ubuntu 24.04 ships 3.2.

# The image Ice's Linux CI uses (packaging/deb/docker/Dockerfile), with all C++, Python and PHP build dependencies.
# The tag is the release channel of main (CHANNEL in config/version.env).
FROM ghcr.io/zeroc-ice/ice-deb-builder-ubuntu24.04:3.9

ENV DEBIAN_FRONTEND=noninteractive \
    LANG=C.UTF-8 \
    OPTIMIZE=no \
    DOTNET_CLI_TELEMETRY_OPTOUT=1 \
    DOTNET_NOLOGO=1

# Java 17 for the Java mapping. The C# mapping targets .NET 8, but Ubuntu's .NET 8 SDK predates support for the
# .slnx solution files the build uses, so it is built with the .NET 10 SDK and the tests run on the .NET 8 runtime.
# openssl and python3-cryptography are needed by the IceSSL tests.
RUN apt-get update && apt-get install -y --no-install-recommends \
      ca-certificates curl xz-utils openjdk-17-jdk-headless dotnet-sdk-10.0 dotnet-runtime-8.0 \
      openssl python3-cryptography \
    && rm -rf /var/lib/apt/lists/*

# Node.js 22, the version the JavaScript CI uses; Ubuntu 24.04 ships Node.js 18.
RUN set -eux; \
    case "$(uname -m)" in x86_64) arch=x64 ;; aarch64) arch=arm64 ;; *) exit 1 ;; esac; \
    base=https://nodejs.org/dist/latest-v22.x; \
    tarball=$(curl -fsSL "$base/SHASUMS256.txt" \
      | awk -v arch="$arch" '$2 ~ "linux-" arch "\\.tar\\.xz$" { print $2 }'); \
    cd /tmp; \
    curl -fsSLO "$base/$tarball"; \
    curl -fsSL "$base/SHASUMS256.txt" | grep " $tarball\$" | sha256sum -c -; \
    tar -xJf "$tarball" -C /usr/local --strip-components=1 --no-same-owner; \
    rm "$tarball"; \
    node --version

# scanner contract: the checkout lives inside the image, at /src
COPY . /src
WORKDIR /src

# Copy the checkout before anything is built in it, for the sanitizer tree.
RUN cp -a /src /opt/ice-asan

# The Slice compilers are built first, as part of `cpp srcs`; every other mapping uses them.
RUN make -j"$(nproc)" -C cpp srcs tests

RUN make -j"$(nproc)" -C python \
    && make -j"$(nproc)" -C php \
    && make -C java \
    && make -C csharp \
    && make -C js

# The Slice compilers run during the build and are sanitized too:
#  - strict_memcmp=0: the system libmcpp (the Slice preprocessor) calls memcmp with a length past the end of its
#    buffer, though the comparison stops before the end. Third-party, and Slice input is trusted.
#  - detect_leaks=0: the compilers exit without freeing everything.
ENV ASAN_OPTIONS=detect_leaks=0:strict_memcmp=0:abort_on_error=1:symbolize=1 \
    UBSAN_OPTIONS=print_stacktrace=1

# The Slice compilers are normally linked statically, which AddressSanitizer does not support, so
# fully_static_ldflags is cleared. The UBSan vptr check is disabled because it makes a plugin reference the typeinfo
# of the classes it uses, and plugins (such as IceGrid registry plugins) are linked with --no-undefined while that
# typeinfo lives in the executable that loads them.
RUN make -j"$(nproc)" -C /opt/ice-asan/cpp \
      CXXFLAGS="-fsanitize=address,undefined -fno-sanitize=vptr -fno-omit-frame-pointer -O1" \
      LDFLAGS="-fsanitize=address,undefined -fno-sanitize=vptr" \
      fully_static_ldflags= \
      srcs tests

# A few cheap suites per mapping, to show the image works. Failures are reported but do not fail the build.
RUN for lang in cpp python php java csharp js; do \
      (cd /src/$lang && python3 allTests.py --filter='Ice/(operations|stream|exceptions)' --continue) \
        || echo "WARNING: $lang tests reported failures"; \
    done; \
    (cd /opt/ice-asan/cpp && python3 allTests.py --filter='Ice/(operations|stream|exceptions)' --continue) \
      || echo "WARNING: sanitizer tests reported failures"
