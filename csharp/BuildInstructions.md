# Building Ice for .NET

This page describes how to build and install Ice for .NET from source code using
Visual Studio. If you prefer, you can also download [binary distributions][1]
for the supported platforms.

## Build Requirements

### Operating Systems and Compilers

Ice for .NET was extensively tested using the operating systems and compiler
versions listed for our [supported platforms][2].

The build requires the [Ice Builder for Visual Studio][8], you must install
version 4.2.0 or greater to build Ice.

### Slice to C# Translator

You will need the Slice to C# translator. ZeroC provides translator binaries for
our supported platforms, or you can build Ice for C++ (which contains the Slice
to C# translator) from source.

## Compiling Ice for .NET with Visual Studio

### Preparing to Build

The build system requires the Slice translator from Ice for C++. If you have not
built Ice for C++ in this source distribution, you must set the `ICE_HOME`
environment variable with the path name of your Ice installation:

    > set ICE_HOME=C:\Program Files (x86)\ZeroC\Ice-3.7a2

### Building Ice for .NET

Open a Visual Studio command prompt and change to the `csharp` subdirectory:

    cd csharp

To build the Ice assemblies, services and tests, run

    Msbuild msbuild\ice.proj

Upon completion, the Ice assemblies are placed in the `Assemblies` subdirectory.

## Running the .NET Tests

Some of the Ice for .NET tests employ applications that are part of the Ice for
C++ distribution. If you have not built Ice for C++ in this source distribution
then you must set the `ICE_HOME` environment variable with the path name of your
Ice installation:

    > set ICE_HOME=C:\Program Files (x86)\ZeroC\Ice-3.7a2

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

## Targeting Managed Code

TODO

## Targeting Unity

TODO

[1]: https://zeroc.com/download.html
[2]: https://doc.zeroc.com/display/Ice37/Supported+Platforms+for+Ice+3.7.0
[3]: https://msdn.microsoft.com/en-us/library/ms241613.aspx
