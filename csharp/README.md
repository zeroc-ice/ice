# Building Ice for C#

This page describes how to build Ice for C# from source.

* [Building on Windows](#building-on-windows)
* [Building on Linux or macOS](#building-on-linux-or-macos)
* [Running the Tests](#running-the-tests)

## Building on Windows
### Windows Build Requirements

You need Visual Studio 2019 (to build the slice2cs compiler written in C++) and the .NET 5.0 SDK.

### Compiling Ice for C# on Windows
Open a Visual Studio command prompt and change to the `csharp` subdirectory:
```
cd csharp
```

Then execute:
```
msbuild msbuild\ice.proj /p:UseSharedCompilation=false /nr:false /m /p:Configuration=Debug
```
for a debug build.

For a release build, execute:
```
msbuild msbuild\ice.proj /p:UseSharedCompilation=false /nr:false /m
```

#### Authenticode Signatures

You can sign the Ice binaries with Authenticode by setting the following environment variables before building these
assemblies:
 - `SIGN_CERTIFICATE` to your Authenticode certificate
 - `SIGN_PASSWORD` to the certificate password

*Authenticode can be generated only from Windows builds.*

## Building on Linux or macOS
### Linux Build Requirements
You need g++ (to build the slice2cs compiler written in C++) and the .NET 5.0 SDK.

### macOS Build Requirements
You need clang (to build the slice2cs compiler written in C++) and the .NET 5.0 SDK.

### Compiling Ice for C# on Linux or macOS

Open a terminal and change to the `csharp` directory:
```
cd csharp
```

Then run:
```
make
```

To create a debug build, set the environment variable OPTIMIZE to no before building, e.g.:
```
export OPTIMIZE=no
make
```

## Running the Tests

Python is required to run the test suite. Additionally, the Glacier2 tests require the Python module `passlib`, which
you can install with the command:
```
pip install passlib
```

To run the tests, open a terminal and change to the top-level directory. At the command prompt, execute:
```
python3 allTests.py
```

On Windows, the command is:
```
python allTests.py --config=Debug --platform=x64
```

If everything worked out, you should see lots of `ok` messages. In case of a failure, the tests abort with `failed`.
