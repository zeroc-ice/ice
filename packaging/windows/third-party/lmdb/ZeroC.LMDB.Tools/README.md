# ZeroC.LMDB.Tools

This package provides the [LMDB] command line tools as x64 executables:

| Tool       | Purpose                                                         |
|------------|-----------------------------------------------------------------|
| `mdb_copy` | Copy or compact an LMDB environment.                            |
| `mdb_dump` | Write the contents of an environment to a portable text format. |
| `mdb_load` | Load an environment from `mdb_dump` output.                     |
| `mdb_stat` | Report environment, database and reader statistics.             |

They are included with the Ice Services installer for Windows, alongside the IceGrid and IceStorm services whose
databases they operate on.

The tools are built with MinGW-w64, because three of them use `getopt`, which the Microsoft CRT does not provide.
They are linked statically and depend on nothing beyond Windows itself.

> **Note for users of `zeroc.lmdb-tools`.** These tools were published as `zeroc.lmdb-tools` up to 0.9.29, and this
> package replaces it. That package will not be updated.
>
> Those builds also used the MSYS toolchain rather than MinGW-w64, so every executable depended on `msys-2.0.dll`,
> which was shipped neither in the package nor in the installer. They could only run on a machine that already had
> MSYS2 installed and on `PATH`. This release fixes that.

## Source

The executables are built from the unmodified upstream LMDB sources. The MSBuild project and nuspec that produce
this package are in [the lmdb directory], and the URL and SHA-256 checksum of the upstream source archive are in
[sources.json].

[the lmdb directory]: https://github.com/zeroc-ice/ice/tree/main/packaging/windows/third-party/lmdb
[sources.json]: https://github.com/zeroc-ice/ice/blob/main/packaging/windows/third-party/sources.json
[LMDB]: https://www.symas.com/mdb/
