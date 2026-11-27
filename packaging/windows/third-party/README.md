# Windows third-party NuGet packages

ZeroC builds and publishes `ZeroC.Bzip2`, `ZeroC.Expat`, `ZeroC.LMDB`, `ZeroC.LMDB.Tools`, `ZeroC.MCPP` and
`ZeroC.OpenSSL` so that Ice users can build Ice C++ from source on Windows without first building these libraries.

This directory contains the projects that build these packages. This page describes how to build and publish them
and is intended for their maintainers; building Ice does not require these steps.

The packages built with Visual Studio use the v142 toolset, the oldest toolset Ice 3.7 supports, and can be used
with v142 and later. One package of each dependency therefore serves every Ice release.

`ZeroC.OpenSSL` is for Ice 3.7 only. On Windows, Ice 3.7 provides an OpenSSL-based IceSSL in addition to its default
Schannel-based IceSSL; Ice 3.8 and later support only Schannel on Windows.

## Build locally

Use Visual Studio with the v142 C++ toolset, PowerShell, Git, tar, CMake and the NuGet CLI. Building LMDB Tools also
requires MinGW-w64 (for example, MSYS2's mingw-w64-x86_64-gcc package). Building OpenSSL also requires a native
Windows Perl such as Strawberry Perl, and NASM, with `perl` and `nasm` in the PATH. From this directory, run:

```shell
msbuild bzip2\bzip2.proj /t:All
msbuild libexpat\expat.proj /t:All
msbuild lmdb\lmdb.proj /t:All
msbuild lmdb\lmdb-tools.proj /t:All
msbuild mcpp\mcpp.proj /t:All
msbuild openssl\openssl.proj /t:All
```

The Build target downloads each pinned upstream archive from [sources.json](sources.json), verifies its SHA-256
checksum and prepares the source. Bzip2's public header is patched before compiling. The NugetPack target copies
the built binaries and upstream license into the package. All runs both targets, and Clean removes build outputs.
Packages are staged under `out/<dependency>/<package>/`; source archives and extracted trees are cached
under `downloads/` and `sources/`.

| Property               | Default     | Purpose                                                |
|------------------------|-------------|--------------------------------------------------------|
| DefaultPlatformToolset | v142        | MSVC toolset for the builds.                           |
| CMakeExe               | cmake       | CMake executable for Expat.                            |
| NuGetExe               | nuget       | NuGet CLI used to pack the binaries.                   |
| MinGWBin               | first found | Directory containing mingw-w64 gcc.exe for LMDB Tools. |
| PerlExe                | perl        | Perl executable for OpenSSL.                           |

## Publish a release

The packages are versioned and released independently of Ice, and are published to nuget.org from main only. One
ZeroC.Expat serves every Ice release, so this directory is not needed on release branches such as 3.8.

[packages.json](packages.json) gives the version of each package, and [sources.json](sources.json) the upstream
version it is built from. They are the same for the first package built from an upstream release. A published
version cannot be replaced, so a package rebuilt from the same sources, for example with another toolset, gets a
new version: the upstream version followed by a revision, such as 1.0.8.1 for the first rebuild of bzip2 1.0.8.
ZeroC's mcpp fork already uses four-part versions, which NuGet cannot extend, so rebuilding ZeroC.MCPP requires a
new tag of the fork.

1. Run **Build Third-Party Windows Packages** manually (Actions tab, Run workflow). It builds, signs and packs the
   package, and attaches the resulting .nupkg to the workflow run as an artifact. Nothing is published yet.
2. Run **Publish Third-Party Windows Packages** with that run's ID and destination `testing`. This pushes the
   artifact to the ZeroC testing feed.
3. Validate the package by building Ice against the testing feed.
4. Run **Publish Third-Party Windows Packages** again with the same run ID and destination `nuget.org`. Only a
   build of main is accepted here.

A build of any branch can be published to `testing`, to try a package update before it is merged.

To add another package, add an entry with its version to [packages.json](packages.json), add its name to the
package choice list of both the build and publish workflows, and provide build and pack projects. Update
[sources.json](sources.json) with the upstream archive URL and verified SHA-256 checksum.
