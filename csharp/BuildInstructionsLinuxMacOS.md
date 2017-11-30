# Building Ice for .NET on Linux and macOS

This page describes how to build and install Ice for .NET from source code using
.NET Core SDK.

## Build Requirements

### Operating Systems and Compilers

.NET Core 2.0 SDK

## Compiling Ice for .NET with .NET Core sdk

Open a command prompt and change to the `csharp` subdirectory:

```
cd csharp
```

To build the Ice assemblies, services and tests, run

```
msbuild msbuild\ice.proj
```

Upon completion, the Ice assemblies for .NET Standard 2.0 are placed in the
`lib\netstandard2.0` directory.

You can skip the build of the test suite with the `BuildDist` target:

```
msbuild msbuild\ice.proj /t:Net45Build
```

## Running the Tests

Python is required to run the test suite. Additionally, the Glacier2 tests
require the Python module `passlib`, which you can install with the command:

```
pip install passlib
```

To run the tests, open a command window and change to the top-level directory.
At the command prompt, execute:

```
python allTests.py
```

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

## NuGet packages

*At this point the creation of NuGet packages is not supported on Linux and macOS*
