# Building Ice for .NET

This page describes how to build Ice for .NET from source and package the
resulting binaries. As an alternative, you can download and install the
[zeroc.ice.net][1] NuGet package.

* [Building on Windows](#building-on-windows)
  * [Windows Build Requirements](#windows-build-requirements)
  * [Compiling Ice for \.NET on Windows](#compiling-ice-for-net-on-windows)
    * [Authenticode Signatures](#authenticode-signatures)
    * [Building only the Test Suite](#building-only-the-test-suite)
* [Building on Linux or macOS](#building-on-linux-or-macos)
  * [Linux and macOS Build Requirements](#linux-and-macos-build-requirements)
  * [Compiling Ice for \.NET on Linux or macOS](#compiling-ice-for-net-on-linux-or-macos)
* [Running the Tests](#running-the-tests)
* [NuGet Package](#nuget-package)

## Building on Windows

### Windows Build Requirements

In order to build Ice for .NET from source, you need:

* Visual Studio 2022 with [.NET 6.0 SDK][2] or [.NET 8.0 SDK][3].

### Compiling Ice for .NET on Windows

Open a Visual Studio command prompt and change to the `csharp` subdirectory:

```shell
cd csharp
```

To build all Ice assemblies and the associated test suite, run:

```shell
msbuild msbuild\ice.proj
```

> Depending on your Visual Studio environment, you may need to specify the platform.
> For example:
>
> ```shell
> msbuild msbuild\ice.proj /p:Platform=x64
> ```

Upon completion, the Ice assemblies for .NET Standard 2.0 are placed in the `lib\netstandard2.0` folder.

You can skip the build of the test suite with the `BuildDist` target:

```shell
msbuild msbuild\ice.proj /t:BuildDist
```

The iceboxnet executable and test applications target `net6.0` You can change
the target framework by setting the `AppTargetFramework` property to a different

Target Framework Moniker value, for example:

```shell
msbuild msbuild\ice.proj /p:"AppTargetFramework=net8.0"
```

This builds the test programs for `net8.0`. The target frameworks you specify
must implement .NET Standard 2.0.

#### Authenticode Signatures

You can sign the Ice binaries with Authenticode by setting the following
environment variables before building these assemblies:

* `SIGN_CERTIFICATE` to your Authenticode certificate
* `SIGN_PASSWORD` to the certificate password
* `SIGN_SHA1` the SHA1 has of the signing certificate

*Authenticode can be generated only from Windows builds.*

#### Building only the Test Suite

You can build only the test suite with this command:

```shell
msbuild msbuild\ice.proj /p:ICE_BIN_DIST=all
```

This build retrieves and installs the `zeroc.ice.net` NuGet package if
necessary.

## Building on Linux or macOS

### Linux and macOS Build Requirements

You need the [.NET 6.0 SDK][2] or [.NET 8.0 SDK][3] to build Ice for .NET from source.

### Compiling Ice for .NET on Linux or macOS

Open a command prompt and change to the `csharp` directory:

```shell
cd csharp
```

Then run:

```shell
dotnet msbuild msbuild/ice.proj
```

Upon completion, the Ice assemblies are placed in the `lib/netstandard2.0` directory.

You can skip the build of the test suite with the `BuildDist` target:

```shell
dotnet msbuild msbuild/ice.proj /t:BuildDist
```

The iceboxnet executable and test applications target `net6.0`. You can change the target
framework by setting the `AppTargetFramework` property to a different Target Framework Moniker value,
for example:

```shell
dotnet msbuild msbuild/ice.proj /p:"AppTargetFramework=net8.0"
```

## Running the Tests

Python is required to run the test suite. Additionally, the Glacier2 tests
require the Python module `passlib`, which you can install with the command:

```shell
pip install passlib
```

To run the tests, open a command window and change to the top-level directory.
At the command prompt, execute:

```shell
python allTests.py
```

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

`allTests.py` executes by default the tests for .NET 6.0. If you want to run
the test with a different .NET Framework you must use `--framework` option.

For example, to run .NET 8.0 tests:

```shell
python allTests.py --framework=net8.0
```

## NuGet Package

### Creating NuGet Packages on Windows

To create a NuGet package, open a Visual Studio command prompt and run the
following command:

```shell
msbuild msbuild\ice.proj /t:NuGetPack
```

This creates the `zeroc.ice.net` Nuget package in the `msbuild\zeroc.ice.net`
directory.

### Creating NuGet Packages on Linux or macOS

To create a NuGet package, open a command prompt and run the following command:

```shell
dotnet msbuild msbuild/ice.proj /t:NuGetPack
```

This creates the `zeroc.ice.net` Nuget package in the `msbuild/zeroc.ice.net`
directory.

[1]: https://zeroc.com/downloads/ice
[2]: https://dotnet.microsoft.com/en-us/download/dotnet/6.0
[3]: https://dotnet.microsoft.com/en-us/download/dotnet/8.0
