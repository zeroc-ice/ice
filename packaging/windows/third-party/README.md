# Windows third-party NuGet packages

We build and publish ZeroC.Bzip2, ZeroC.Expat, ZeroC.LMDB and ZeroC.LMDB.Tools so that we and our users can build
Ice C++ from source on Windows without first building these libraries.

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

Build the package with the manually dispatched **Build Third-Party Windows Packages** workflow. It builds, signs
where applicable, packs and uploads the NuGet artifact. Dispatch **Publish Third-Party Windows Packages** with that
build's run ID and destination testing. Validate the package by building Ice against it from the testing feed, then
publish the validated artifact to nuget.org. A build of any branch can be published to testing, so a package update
can be tried before it reaches main; nuget.org takes builds of main only. Packages are versioned and released
independently of Ice itself.

To add another package, add an entry to [packages.json](packages.json), add its name to the package choice list of
both dispatch workflows, and provide build and pack projects. Update [sources.json](sources.json) with the upstream
archive URL and verified SHA-256 checksum.
