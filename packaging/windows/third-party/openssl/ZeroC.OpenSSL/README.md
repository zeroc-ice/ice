# ZeroC.OpenSSL

This package provides the [OpenSSL] debug and release DLLs (`libcrypto` and `libssl`), their corresponding import
libraries, the OpenSSL header files, and the `openssl` command-line tool.

The DLLs are built with the v142 platform toolset and can be used from projects built with v142 or later.

## Source

The binaries are built from the unmodified upstream OpenSSL sources. The MSBuild projects, nuspec and targets that
produce this package are in [the openssl directory], and the URL and SHA-256 checksum of the upstream source archive
are in [sources.json].

[the openssl directory]: https://github.com/zeroc-ice/ice/tree/main/packaging/windows/third-party/openssl
[sources.json]: https://github.com/zeroc-ice/ice/blob/main/packaging/windows/third-party/sources.json
[OpenSSL]: https://openssl-library.org/
