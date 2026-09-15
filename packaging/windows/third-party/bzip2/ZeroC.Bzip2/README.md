# ZeroC.Bzip2

This package provides the [Bzip2] debug and release DLLs, their corresponding import libraries, and the bzip2 header
file. Both the shared and static CRT variants are included.

## Source Code

The binaries are built from the upstream bzip2 sources with one patch to `bzlib.h`. The MSBuild project, nuspec
and targets that produce this package are in [the bzip2 directory], the patch is in [patches/bzip2], and the URL
and SHA-256 checksum of the upstream source archive are in [sources.json].

[the bzip2 directory]: https://github.com/zeroc-ice/ice/tree/main/packaging/windows/third-party/bzip2
[patches/bzip2]: https://github.com/zeroc-ice/ice/tree/main/packaging/windows/third-party/patches/bzip2
[sources.json]: https://github.com/zeroc-ice/ice/blob/main/packaging/windows/third-party/sources.json
[Bzip2]: https://sourceware.org/bzip2/
