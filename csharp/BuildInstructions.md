# Building Ice for .NET

This page describes how to build and install Ice for .NET from source code using
Visual Studio. If you prefer, you can also download [binary distributions][1]
for the supported platforms.

## Build Requirements

### Operating Systems and Compilers

Ice for .NET was extensively tested using the operating systems and compiler
versions listed for our [supported platforms][2].

### Slice to C# Translator

You will need the Slice to C# translator. ZeroC provides translator binaries for
our supported platforms, or you can build Ice for C++ (which contains the Slice
to C# translator) from source.
 
## Compiling Ice for .NET with Visual Studio

### Preparing to Build

The build system requires the Slice translator from Ice for C++. If you have not
built Ice for C++ in this source distribution, you must set the `ICE_HOME`
environment variable with the path name of your Ice installation:

    > set ICE_HOME=C:\Program Files (x86)\ZeroC\Ice-3.6.4

### Building Ice for .NET

Open a Visual Studio command window and change to the `csharp` subdirectory:

    > cd csharp

Review the settings in the file `config\Make.rules.mak.cs` and edit as
necessary. For example, you may wish to enable optimization.

To build the Ice assemblies, services and tests, run

    > nmake /f Makefile.mak

Upon completion, the Ice assemblies are placed in the `Assemblies` subdirectory.

## Running the .NET Tests

Some of the Ice for .NET tests employ applications that are part of the Ice for
C++ distribution. If you have not built Ice for C++ in this source distribution
then you must set the `ICE_HOME` environment variable with the path name of your
Ice installation:

    > set ICE_HOME=C:\Program Files (x86)\ZeroC\Ice-3.6.4

Python is required to run the test suite. Additionally, the Glacier2 tests
require the Python module `passlib`, which you can install with the command:

    > pip install passlib

To run the tests, open a command window and change to the top-level directory.
At the command prompt, execute:

    > python allTests.py

You can also run tests individually by changing to the test directory and
running this command:

    > python run.py

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

## Protocol Compression with .NET

Ice for .NET attempts to dynamically load `bzip2.dll` to support protocol
compression, therefore this DLL must be present in your PATH. Ice automatically
disables protocol compression if the DLL cannot be found.

On 64-bit Windows, you must ensure that Ice finds the 64-bit version of
`bzip2.dll` before the 32-bit version. The 64-bit and 32-bit bzip2 libraries are
installed in `<prefix>\bin\x64` and `<prefix>\bin`, respectively. For 64-bit
Windows, the `<prefix>\bin\x64` directory must appear before `<prefix>\bin` in
your application's PATH. (The Ice run time prints a warning to the console if it
detects a `bzip2.dll` format mismatch during start-up.)

## Installing Ice for .NET

Run `nmake /f Makefile.mak install` to install Ice for .NET in the directory
specified by the `prefix` variable in `config\Make.rules.mak.cs`. After
installation, the `<prefix>\bin` directory contains executables (such as
`iceboxnet.exe`), and the `<prefix>\Assemblies` directory contains the .NET
assemblies.

> *This command must be executed in a command prompt that has administrative
privileges because it requires write access to the registry to register the
assemblies.*

## GAC Installation

You can add the assemblies to the Global Assembly Cache (GAC). To do this, open
Windows Explorer and navigate to the directory `C:\WINDOWS\assembly`. Next, drag
and drop (or copy and paste) the assemblies from the `Assemblies` subdirectory
of your Ice source build into the right-hand pane to install them in the cache.

You can also use `gacutil` from the command line to achieve the same result:

    > gacutil /i library.dll

The `gacutil` tool should already be in your PATH if you open a Visual Studio
command prompt.

Once installed in the cache, the assemblies will always be located correctly
without having to set environment variables or copy them into the same directory
as an executable.

If you want line numbers for stack traces, you will also need to deploy the Ice
PDB files alongside the assemblies or configure Visual Studio to locate them.

## Targeting Managed Code

By default, Ice for .NET uses unmanaged code for performing protocol compression
and for handling signals in the `Ice.Application` class. You can build a managed
version of Ice for .NET that lacks the aforementioned features by editing
`config/Make.rules.mak.cs` and uncommenting the `MANAGED=yes` line before you
build. Run nmake to start the build:

    > nmake /f Makefile.mak

## Targeting Unity

Ice for .NET can also be compiled to target the Unity API. To build Ice for the
Unity API, open `config\Make.rules.mak.cs` and set `UNITY=yes`. Run nmake as
before:

    > nmake /f Makefile.mak

[1]: https://zeroc.com/download.html
[2]: https://doc.zeroc.com/display/Ice36/Supported+Platforms+for+Ice+3.6.4
[3]: https://msdn.microsoft.com/en-us/library/ms241613.aspx
