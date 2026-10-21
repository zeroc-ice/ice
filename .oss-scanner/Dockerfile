# OSS Scanner build of Ice (https://github.com/zeroc-ice/ice).
#
# The scanner builds this file with the Ice checkout as the build context. The network is available only while this
# file builds; the scan itself runs offline, so every toolchain and package dependency is fetched here.
#
# Two trees are built from the same checkout:
#
#   /src            The checkout. C++ (sources and tests), Python, PHP, Ruby, Java, C# and JavaScript, built with
#                   OPTIMIZE=no: debug info, no optimization, and asserts enabled. Run tests from here, for example
#                   `python3 cpp/allTests.py --filter=Ice/stream`, or `python3 allTests.py --filter=...` in another
#                   language directory.
#   /opt/ice-asan   A copy of the checkout with only C++ (sources and tests), built with AddressSanitizer and
#                   UndefinedBehaviorSanitizer. Use it to confirm memory-safety findings:
#                   `python3 /opt/ice-asan/cpp/allTests.py --filter=Ice/stream`.
#
# Swift (Apple platforms only) and MATLAB (proprietary) are not built.

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

# Ruby 3.4, the minimum Ice for Ruby supports; Ubuntu 24.04 ships Ruby 3.2. This is the ruby/ruby-builder build that
# ruby/setup-ruby installs in CI. It is not relocatable, so it goes in the same prefix as on the CI runners. To update
# it, change RUBY_VERSION and the two SHA-256 digests listed on the release page.
ARG RUBY_VERSION=3.4.11
RUN set -eux; \
    case "$(uname -m)" in \
      x86_64) arch=x64; sha256=e3d114c546332243f84755aa277ba1e1781deba686050d729d2806d3766baf25 ;; \
      aarch64) arch=arm64; sha256=cb16027d1db27dcf550dfd03486b8f7ef7b0a75e5127caa15bb3dd05e40a6eda ;; \
      *) exit 1 ;; \
    esac; \
    tarball="ruby-$RUBY_VERSION-ubuntu-24.04-$arch.tar.gz"; \
    cd /tmp; \
    curl -fsSLO "https://github.com/ruby/ruby-builder/releases/download/ruby-$RUBY_VERSION/$tarball"; \
    echo "$sha256  $tarball" | sha256sum -c -; \
    mkdir -p "/opt/hostedtoolcache/Ruby/$RUBY_VERSION"; \
    tar -xzf "$tarball" -C "/opt/hostedtoolcache/Ruby/$RUBY_VERSION"; \
    rm "$tarball"; \
    ln -s "/opt/hostedtoolcache/Ruby/$RUBY_VERSION/$arch" /opt/ruby; \
    /opt/ruby/bin/ruby --version

ENV PATH=/opt/ruby/bin:$PATH

# scanner contract: the checkout lives inside the image, at /src
COPY . /src
WORKDIR /src

# Copy the checkout before anything is built in it, for the sanitizer tree.
RUN cp -a /src /opt/ice-asan

# The Slice compilers are built first, as part of `cpp srcs`; every other mapping uses them.
RUN make -j"$(nproc)" -C cpp srcs tests

RUN make -j"$(nproc)" -C python \
    && make -j"$(nproc)" -C php \
    && make -j"$(nproc)" -C ruby \
    && make -C java \
    && make -C csharp \
    && make -C js

# The Slice compilers are built first and without sanitizers: they compile trusted input, the system libmcpp they
# use (the Slice preprocessor) trips AddressSanitizer's memcmp check, and they exit without freeing everything. The
# sanitized build below then finds them up to date, so they stay unsanitized when tests or Slice files are rebuilt.
RUN make -j"$(nproc)" -C /opt/ice-asan/cpp $(cd /opt/ice-asan/cpp && ls -d src/slice2*)

ENV ASAN_OPTIONS=abort_on_error=1:symbolize=1 \
    UBSAN_OPTIONS=print_stacktrace=1

# The UBSan vptr check is disabled because it makes a plugin reference the typeinfo of the classes it uses, and plugins
# (such as IceGrid registry plugins) are linked with --no-undefined while that typeinfo lives in the executable that
# loads them.
RUN make -j"$(nproc)" -C /opt/ice-asan/cpp \
      CXXFLAGS="-fsanitize=address,undefined -fno-sanitize=vptr -fno-omit-frame-pointer -O1" \
      LDFLAGS="-fsanitize=address,undefined -fno-sanitize=vptr" \
      srcs tests

# A few cheap suites per mapping, to show the image works. Failures are reported but do not fail the build.
RUN for lang in cpp python php ruby java csharp js; do \
      (cd /src/$lang && python3 allTests.py --filter='Ice/(operations|stream|exceptions)' --continue) \
        || echo "WARNING: $lang tests reported failures"; \
    done; \
    (cd /opt/ice-asan/cpp && python3 allTests.py --filter='Ice/(operations|stream|exceptions)' --continue) \
      || echo "WARNING: sanitizer tests reported failures"
