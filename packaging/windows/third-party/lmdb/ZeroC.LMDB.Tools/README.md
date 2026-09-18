# ZeroC.LMDB.Tools

This package provides the [LMDB] command line tools as x64 executables:

| Tool       | Purpose                                                         |
|------------|-----------------------------------------------------------------|
| `mdb_copy` | Copy or compact an LMDB environment.                            |
| `mdb_dump` | Write the contents of an environment to a portable text format. |
| `mdb_load` | Load an environment from `mdb_dump` output.                     |
| `mdb_stat` | Report environment, database and reader statistics.             |

The tools are built with MinGW-w64 and linked statically.

## Source

The executables are built from the unmodified upstream LMDB sources. The MSBuild project and nuspec that produce
this package are in [the lmdb directory], and the URL and SHA-256 checksum of the upstream source archive are in
[sources.json].

[the lmdb directory]: https://github.com/zeroc-ice/ice/tree/main/packaging/windows/third-party/lmdb
[sources.json]: https://github.com/zeroc-ice/ice/blob/main/packaging/windows/third-party/sources.json
[LMDB]: https://www.symas.com/mdb/
