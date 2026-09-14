# ZeroC.Bzip2

This package provides the [Bzip2] debug and release DLLs, their corresponding import libraries, and the bzip2 header
file. Both the shared and static CRT variants are included.

## Source Code

bzip2 is used as released, with one patch to `bzlib.h` that replaces its dynamic-loading declarations with
`__declspec(dllexport)` and `__declspec(dllimport)`, so the DLL can be linked in the ordinary way. The patch, the
MSBuild project, the nuspec and the targets used to produce this package are available on GitHub, alongside the
source URL and checksum for the upstream release the binaries were built from:

👉 https://github.com/zeroc-ice/ice/tree/main/packaging/third-party/windows

[Bzip2]: https://sourceware.org/bzip2/
