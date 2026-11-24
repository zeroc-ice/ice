# ZeroC.MCPP

This package provides the [mcpp] static libraries for both debug and release builds. Both the shared and static CRT
variants are included.

## Source

The libraries are built from the unmodified sources of ZeroC's mcpp fork, which is mcpp 2.7.2 simplified for the
Ice Slice compilers. The MSBuild project, nuspec and targets that produce this package are in [the mcpp directory],
and the URL and SHA-256 checksum of the source archive are in [sources.json].

[the mcpp directory]: https://github.com/zeroc-ice/ice/tree/main/packaging/windows/third-party/mcpp
[sources.json]: https://github.com/zeroc-ice/ice/blob/main/packaging/windows/third-party/sources.json
[mcpp]: https://github.com/zeroc-ice/mcpp
