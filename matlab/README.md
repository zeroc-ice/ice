# Building Ice for MATLAB

This document describes how to build and install Ice for MATLAB from source.

ZeroC provides Ice for MATLAB [toolboxes][1] for MATLAB on Windows and Linux, so building Ice for MATLAB from source is
usually unnecessary.

* [Building Ice for MATLAB on Windows](#building-ice-for-matlab-on-windows)
  * [Prerequisites](#prerequisites)
  * [Build Instructions](#build-instructions)
  * [Packaging the Ice Toolbox](#packaging-the-ice-toolbox)
* [Building Ice for MATLAB on Linux](#building-ice-for-matlab-on-linux)
  * [Prerequisites](#prerequisites-1)
  * [Build Instructions](#build-instructions-1)
* [Using Ice for MATLAB](#using-ice-for-matlab)
  * [Search Path](#search-path)
  * [Slice Files](#slice-files)
  * [Loading the Library](#loading-the-library)
  * [Running the Tests](#running-the-tests)

## Building Ice for MATLAB on Windows

### Prerequisites

The build system requires MATLAB 2016a or later for Windows, Visual Studio 2015
and a Perl installation.

### Build Instructions

Open a Visual Studio x64 command prompt, `VS2015 x64 Native Tools Command Prompt`. In this Command Prompt, change to the
`matlab` subdirectory:

```
cd matlab
```

Now you're ready to build Ice for MATLAB:
```
msbuild msbuild\ice.proj
```

To build in debug mode instead:
```
msbuild msbuild\ice.proj /p:Configuration=Debug
```

Upon completion, a build in release mode generates the following components:

 - Ice for C++11 libraries, located in `cpp\bin\x64\Release`
 - slice2matlab executable, located in `cpp\bin\x64\Release`
 - ice.mexw64 MEX file, located in `matlab\lib\x64\Release`
 - Prototype and thunk files, located in `matlab\lib\x64\Release`
 - MATLAB code for core Slice files, located in `matlab\lib\generated`
 - MATLAB code for test Slice files, located in `matlab\test\**\generated`

The MATLAB extension depends on Ice for C++ components from the `cpp`
subdirectory, and those are built if required. It is also possible to build the
MATLAB extension using Ice C++ NuGet packages by setting the `ICE_BIN_DIST`
msbuild property to `cpp`:

```
msbuild msbuild\ice.proj /p:ICE_BIN_DIST=cpp
```

### Packaging the Ice Toolbox

Use the following command to build the Ice toolbox package:
```
msbuild msbuild\ice.proj /t:Package
```

This creates the toolbox package `toolbox\ice-<Ice Version>-<MATLAB Version>-win.mltbx`.

You can install the toolbox from within MATLAB by double-clicking on the file.

## Building Ice for MATLAB on Linux

### Prerequisites

The build system requires MATLAB 2017b or MATLAB 2019b for Linux, with a supported C++ compiler and a Perl installation.

* Use GCC 4.9 for 2017b, see [MATLAB 2017b supported compilers][2].
* Use GCC 6.3 for 2019b, see [MATLAB 2019b supported compilers][3].

We recommend using the following build environments:

* Ubuntu 16.04 (Xenial) with g++-4.9 compiler, for MATLAB 2017b.
* Debian 9 (Stretch) with the default g++ compiler, for MATLAB 2019b.

### Build Instructions

The MATLAB extension depends on Ice for C++ components from the cpp subdirectory, and those need to be built, using the
`cpp11-shared` configuration, if you have not built the C++ distribution first review [cpp/README.md](../cpp/README.md).

In a command window, change to the matlab subdirectory:

```
cd matlab
```

Review the [matlab/config/Make.rules](config/Make.rules) in your build tree and update the configuration if needed. The
comments in the file provide more information.

Run `make` to build the Ice for MATLAB toolbox and the MATLAB tests. Set `V=1` to get a more detailed build output.

```
make
```

This creates the toolbox package `toolbox/ice-<Ice Version>-<MATLAB Version>-linux.mltbx`. You can install the toolbox from
within MATLAB by double-clicking on the file.

To build the Ice for MATLAB toolbox using the C++ binary distribution, set `ICE_BIN_DIST=cpp`, note this is only possible
with the MATLAB 2019b builds, because MATLAB 2017b needs to use the g++-4.9 that is not compatible with the Xenial
binary distribution.

When building for MATALB 2017b you must set `CC` and `CXX` variables to use the `gcc-4.9` and `g++-4.9` compilers
respectively.

```
CC=gcc-4.9
CXX=g++-4.9
```

## Using Ice for MATLAB

### Search Path

To use a source build, add the following directories to your MATLAB path:
 - `matlab\lib`
 - `matlab\lib\generated`
 - `matlab\lib\x64\Release` (only on Windows platforms)
 - `matlab\lib\x86_64-linux-gnu` (only on Linux platforms)

### Slice Files

Use `slice2matlab` to compile your Slice files. Run `slice2matlab -h` for a description of its command-line options. You
can place the generated `*.m` files anywhere you like, but the enclosing directory must be in your MATLAB path.

### Loading the Library

The Ice for MATLAB library can be loaded with this command:
```
loadlibrary('ice', @iceproto)
```

### Running the Tests

The Ice for MATLAB tests are located in `matlab\test`.

Since Ice for MATLAB only supports client functionality, you need test servers from a different language mapping.

Assuming you've installed Ice for Python, run `allTests.py`:

```
python allTests.py
```

This script automatically starts a Python server for each MATLAB client. MATLAB clients are executed using a minimized
MATLAB interpreter and the test output is copied to the Command Prompt window.

[1]: https://zeroc.com/downloads/ice
[2]: https://www.mathworks.com/content/dam/mathworks/mathworks-dot-com/support/sysreq/files/SystemRequirements-Release2017b_SupportedCompilers.pdf
[3]: https://www.mathworks.com/content/dam/mathworks/mathworks-dot-com/support/sysreq/files/system-requirements-release-2019b-supported-compilers.pdf
