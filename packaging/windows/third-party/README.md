# Windows third-party NuGet packages

ZeroC builds and publishes `ZeroC.Bzip2`, `ZeroC.Expat`, `ZeroC.LMDB` and `ZeroC.LMDB.Tools` so that Ice users can
build Ice C++ from source on Windows without first building these libraries.

## Build locally

Use Visual Studio with the v143 C++ toolset, PowerShell, Git, tar, CMake and the NuGet CLI. Building LMDB Tools also
requires MinGW-w64 (for example, MSYS2's mingw-w64-x86_64-gcc package). From this directory, run:

```shell
msbuild bzip2\bzip2.proj /t:All
msbuild libexpat\expat.proj /t:All
msbuild lmdb\lmdb.proj /t:All
msbuild lmdb\lmdb-tools.proj /t:All
```

The Build target downloads each pinned upstream archive from [sources.json](sources.json), verifies its SHA-256
checksum and prepares the source. Bzip2's public header is patched before compiling. The NugetPack target copies
the built binaries and upstream license into the package. All runs both targets, and Clean removes build outputs.
Packages are staged under `out/<dependency>/<package>/`; source archives and extracted trees are cached
under `downloads/` and `sources/`.

| Property               | Default     | Purpose                                                |
|------------------------|-------------|--------------------------------------------------------|
| DefaultPlatformToolset | v143        | MSVC toolset for DLL and library builds.               |
| CMakeExe               | cmake       | CMake executable for Expat.                            |
| NuGetExe               | nuget       | NuGet CLI used to pack the binaries.                   |
| MinGWBin               | first found | Directory containing mingw-w64 gcc.exe for LMDB Tools. |

## Publish a release

The packages are versioned and released independently of Ice, and are published to nuget.org from main only. One
ZeroC.Expat serves every Ice release, so this directory is not needed on release branches such as 3.8.

1. Run **Build Third-Party Windows Packages** manually (Actions tab, Run workflow). It builds, signs and packs the
   package, and attaches the resulting .nupkg to the workflow run as an artifact. Nothing is published yet.
2. Run **Publish Third-Party Windows Packages** with that run's ID and destination `testing`. This pushes the
   artifact to the ZeroC testing feed.
3. Validate the package by building Ice against the testing feed.
4. Run **Publish Third-Party Windows Packages** again with the same run ID and destination `nuget.org`. Only a
   build of main is accepted here.

A build of any branch can be published to `testing`, to try a package update before it is merged.

To add another package, add an entry to [packages.json](packages.json), add its name to the package choice list of
both the build and publish workflows, and provide build and pack projects. Update [sources.json](sources.json) with
the upstream archive URL and verified SHA-256 checksum.
