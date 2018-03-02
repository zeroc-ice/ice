# Building Ice for MATLAB

This document describes how to build and install Ice for MATLAB from source.

ZeroC provides Ice for MATLAB [toolboxes][1] for all recent versions of MATLAB on Windows,
so building Ice for MATLAB from source is usually unnecessary.

* [Prerequisites](#prerequisites)
* [Build Instructions](#build-instructions)
* [Packaging the Ice Toolbox](#packaging-the-ice-toolbox)
* [Using Ice for MATLAB](#using-ice-for-matlab)
  * [Search Path](#search-path)
  * [Slice Files](#slice-files)
  * [Loading the Library](#loading-the-library)
  * [Running the Tests](#running-the-tests)
    * [Running the Tests Manually](#running-the-tests-manually)
    * [Running the Automated Tests](#running-the-automated-tests)

### Prerequisites

The build system requires MATLAB 2016a or later for Windows, Visual Studio 2015 
and a Perl installation.

### Build Instructions

Open a Visual Studio x64 command prompt, `VS2015 x64 Native Tools Command
Prompt`. In this Command Prompt, change to the `matlab` subdirectory:
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

### Packaging the Ice Toolbox

Use the following command to build the Ice toolbox package:
```
msbuild msbuild\ice.proj /t:Package
```

This creates the toolbox package `msbuild\ice-<Ice Version>-<MATLAB Version>.mltbx`.

You can install the package from within MATLAB by double-clicking on the file.

### Using Ice for MATLAB

#### Search Path

To use the source build, add the following directories to your MATLAB path:

 - `matlab\lib`
 - `matlab\lib\generated`
 - `matlab\lib\x64\Release`

#### Slice Files

Use `slice2matlab` to compile your Slice files. Run `slice2matlab -h` for a
description of its command-line options. You can place the generated `*.m`
files anywhere you like, but the enclosing directory must be in your MATLAB
path.

#### Loading the Library

The Ice for MATLAB library can be loaded with this command:
```
loadlibrary('ice', @iceproto)
```

The MEX file depends on `bzip2.dll` and several Ice DLLs that are part of the
Ice for C++ distribution. The build copies all DLL dependencies to the
`matlab\lib\x64\Release` directory.

#### Running the Tests

The Ice for MATLAB tests are located in `matlab\test`.

Since Ice for MATLAB only supports client functionality, you will have to build
test servers from a different language mapping.

##### Running the Tests Manually

In a Command Prompt, start a test server from your chosen server language
mapping.

In MATLAB, change to a test directory:
```
cd matlab\test\...
```

Now you can start the MATLAB test client. Assuming the server is running on
the same host, use this command:
```
client({})
```

If you started the server on a different host, use this command instead:
```
client({'--Ice.Default.Host=<addr>'})
```

Replace `<addr>` with the host name or IP address of the server host.

##### Running the Automated Tests

Assuming you've built the C++11 test servers in Release mode on Windows, run the
`allTests.py` script like this:

```
python allTests.py --platform=x64 --cpp-config=Cpp11-Release
```

This command will automatically start a corresponding C++11 server for each
MATLAB client. MATLAB clients are executed using a minimized MATLAB interpreter
and the test output is copied to the Command Prompt window.

[1]: https://zeroc.com/downloads/ice#matlab
