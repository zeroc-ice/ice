# Building Ice for C#

This page describes how to build Ice for C# from source.

* [Linux Build Requirements](#linux-build-requirements)
* [macOS Build Requirements](#macos-build-requirements)
* [Windows Build Requirements](#windows-build-requirements)
* [Build Command](#build-command)
* [Running the Tests](#running-the-tests)

## Linux Build Requirements
You need g++ (to build the slice2cs compiler written in C++) and the .NET 5.0 SDK.

## macOS Build Requirements
You need clang (to build the slice2cs compiler written in C++) and the .NET 5.0 SDK.

## Windows Build Requirements
You need Visual Studio 2019 (to build the slice2cs compiler written in C++) and the .NET 5.0 SDK.

## Build Command

In a terminal (Linux, macOS) or in a Visual Studio command prompt (Windows), go to the `csharp` subdirectory:
```
cd csharp
```

Then execute:
```
dotnet msbuild msbuild/ice.proj /m /p:Configuration=Debug
```
for a debug build.

For a release build, execute:
```
dotnet msbuild msbuild/ice.proj /m
```

### Authenticode Signatures

You can sign the Ice binaries with Authenticode by setting the following environment variables before building these
assemblies:
 - `SIGN_CERTIFICATE` to your Authenticode certificate
 - `SIGN_PASSWORD` to the certificate password

*Authenticode can be generated only from Windows builds.*

## Running the Tests

Python is required to run the test suite. Additionally, the Glacier2 tests require the Python module `passlib`, which
you can install with the command:
```
python3 -m pip install passlib
```

To run the tests, open a terminal and change to the top-level directory. At the command prompt, execute:
```
python3 allTests.py
```

On Windows, the command is:
```
python allTests.py --config=Debug --platform=x64
```

This executes all the tests for the ice2 protocol using the tcp transport. Add `--all` to execute the tests for both
ice1 and ice2 with various transports. Pass `--help` to allTests.py to see additional options.

If everything worked out, you should see lots of `ok` messages. In case of a failure, the tests abort with `failed`.
