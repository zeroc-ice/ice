# Building Ice for .NET

This page describes how to build Ice for .NET from source and package the resulting binaries. As an alternative, you
can download and install the [zeroc.ice.net][1] NuGet package.

* [Building on Windows](#building-on-windows)
  * [Windows Build Requirements](#windows-build-requirements)
  * [Compiling Ice for \.NET on Windows](#compiling-ice-for-net-on-windows)
    * [Authenticode Signatures](#authenticode-signatures)
* [Building on Linux or macOS](#building-on-linux-or-macos)
  * [Linux and macOS Build Requirements](#linux-and-macos-build-requirements)
  * [Compiling Ice for \.NET on Linux or macOS](#compiling-ice-for-net-on-linux-or-macos)
* [Running the Tests](#running-the-tests)
* [NuGet Package](#nuget-package)

## Building on Windows

### Windows Build Requirements

In order to build Ice for .NET from source, you need:

* Visual Studio 2022 with [.NET 8.0 SDK][2].

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

Upon completion, the Ice assemblies for .NET 8.0 are placed in the `lib\net8.0` folder.

#### Authenticode Signatures

You can sign the Ice binaries with Authenticode by setting the following
environment variables before building these assemblies:

* `SIGN_CERTIFICATE` to your Authenticode certificate
* `SIGN_PASSWORD` to the certificate password
* `SIGN_SHA1` the SHA1 has of the signing certificate

*Authenticode can be generated only from Windows builds.*

## Building on Linux or macOS

### Linux and macOS Build Requirements

You need the [.NET 8.0 SDK][2] to build Ice for .NET from source.

### Compiling Ice for .NET on Linux or macOS

Open a command prompt and change to the `csharp` directory:

```shell
cd csharp
```

Then run:

```shell
dotnet msbuild msbuild/ice.proj
```

Upon completion, the Ice assemblies are placed in the `lib/net8.0` directory.

## Running the Tests

Python is required to run the test suite. Additionally, the Glacier2 tests require the Python module `passlib`, which
you can install with the command:

```shell
pip install passlib
```

To run the tests, open a command window and change to the top-level directory. At the command prompt, execute:

```shell
python allTests.py
```

If everything worked out, you should see lots of `ok` messages. In case of a failure, the tests abort with `failed`.

## NuGet Package

### Creating NuGet Packages on Windows

To create a NuGet package, open a Visual Studio command prompt and run the following command:

```shell
msbuild msbuild\ice.proj /t:NuGetPack
```

This creates the `zeroc.ice.net` Nuget package in the `msbuild\zeroc.ice.net` directory.

You can publish the package to your local `global-packages` source with the following command:

```shell
msbuild msbuild\ice.proj /t:Publish
```

### Creating NuGet Packages on Linux or macOS

To create a NuGet package, open a command prompt and run the following command:

```shell
dotnet msbuild msbuild/ice.proj /t:NuGetPack
```

This creates the `zeroc.ice.net` Nuget package in the `msbuild/zeroc.ice.net` directory.

You can publish the package to your local `global-packages` source with the following command:

```shell
dotnet msbuild msbuild\ice.proj /t:Publish
```

[1]: https://zeroc.com/downloads/ice
[2]: https://dotnet.microsoft.com/en-us/download/dotnet/8.0
