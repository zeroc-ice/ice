# Building Ice for .NET

This page describes how to build Ice for .NET from source and package the
resulting binaries. As an alternative, you can download and install the
[zeroc.ice.net][1] NuGet package.

* [Building on Windows](#building-on-windows)
  * [Windows Build Requirements](#windows-build-requirements)
  * [Compiling Ice for \.NET on Windows](#compiling-ice-for-net-on-windows)
    * [Strong Name Signatures for \.NET Framework 4\.5 Assemblies](#strong-name-signatures-for-net-framework-45-assemblies)
    * [Authenticode Signatures](#authenticode-signatures)
    * [Building only the Test Suite](#building-only-the-test-suite)
* [Building on Linux or macOS](#building-on-linux-or-macos)
  * [Linux and macOS Build Requirements](#linux-and-macos-build-requirements)
  * [Compiling Ice for \.NET on Linux or macOS](#compiling-ice-for-net-on-linux-or-macos)
* [Running the Tests](#running-the-tests)
* [NuGet Package](#nuget-package)
* [Building Ice for Xamarin Test Suite](#building-ice-for-xamarin-test-suite)

## Building on Windows

A source build of Ice for .NET on Windows produces two sets of assemblies:
 - assemblies for the .NET Framework 4.5
 - assemblies for [.NET Standard 2.0][2]

### Windows Build Requirements

In order to build Ice for .NET from source, you need:
 - A [supported version][3] of Visual Studio when building .NET Framework 4.5 Assemblies.
 - Visual Studio 2022 with [.NET 6.0 SDK][4] or [.NET 7.0 SDK][5] when building the .NET Standard 2.0 Assemblies.
 - Visual Studio 2022 with [.NET 6.0 SDK][4] and [.NET 7.0 SDK][5] when building the NuGet packages.

### Compiling Ice for .NET on Windows

Open a Visual Studio command prompt and change to the `csharp` subdirectory:
```
cd csharp
```

To build all Ice assemblies and the associated test suite, run:
```
msbuild msbuild\ice.proj
```

Upon completion, the Ice assemblies for the .NET Framework 4.5 and .NET Standard 2.0 are placed
in the `lib\net45` and `lib\netstandard2.0` folders respectively.

You can skip the build of the test suite with the `BuildDist` target:
```
msbuild msbuild\ice.proj /t:BuildDist
```

The `Net45Build`, `Net45BuildDist`, `NetStandardBuild` and `NetStandardBuildDist` targets allow
you to build assemblies only for the .NET Framework 4.5 or .NET Standard 2.0, with or without
the test suite.

> Note: Visual Studio 2022 version or higher is required for .NET Standard 2.0 builds.

The .NET Standard build of iceboxnet and test applications target `net6.0` You can change
the target framework by setting the `AppTargetFramework` property to a different

Target Framework Moniker value, for example:

```
msbuild msbuild\ice.proj /p:"AppTargetFramework=net7.0"
```

This builds the test programs for `net7.0`. The target frameworks you specify
must implement .NET Standard 2.0.

#### Strong Name Signatures

You can add Strong Naming signatures to the Ice assemblies by setting the
following environment variables before building these assemblies:

 - `PUBLIC_KEYFILE` Identity public key used to delay sign the assembly
 - `KEYFILE` Identity full key pair used to sign the assembly

If only `PUBLIC_KEYFILE` is set, the assemblies are delay-signed during the
build and you must re-sign the assemblies later with the full identity key pair.

If only `KEYFILE` is set, the assemblies are fully signed during the build using
`KEYFILE`.

If both `PUBLIC_KEYFILE` and `KEYFILE` are set, assemblies are delay-signed
during the build using `PUBLIC_KEYFILE` and re-signed after the build using
`KEYFILE`. This can be used for generating [Enhanced Strong Naming][6]
signatures.

*Strong Name Signatures can be generated only from Windows builds.*

#### Authenticode Signatures

You can sign the Ice binaries with Authenticode by setting the following
environment variables before building these assemblies:
 - `SIGN_CERTIFICATE` to your Authenticode certificate
 - `SIGN_PASSWORD` to the certificate password
 - `SIGN_SHA1` the SHA1 has of the signing certificate

*Authenticode can be generated only from Windows builds.*

#### Building only the Test Suite

You can build only the test suite with this command:
```
msbuild msbuild\ice.proj /p:ICE_BIN_DIST=all
```

This build retrieves and installs the `zeroc.ice.net` NuGet package if
necessary.

## Building on Linux or macOS

### Linux and macOS Build Requirements

You need the [.NET 6.0 SDK][4] or [.NET 7.0 SDK][5] to build Ice for .NET from source.

### Compiling Ice for .NET on Linux or macOS

Open a command prompt and change to the `csharp` directory:
```
cd csharp
```

Then run:
```
dotnet msbuild msbuild/ice.proj
```

Upon completion, the Ice assemblies for .NET Standard 2.0 are placed in the `lib/netstandard2.0`
directory.

You can skip the build of the test suite with the `BuildDist` target:
```
dotnet msbuild msbuild/ice.proj /t:BuildDist
```

The .NET Standard build of iceboxnet and test applications target `net6.0`. You can change the target
framework by setting the `AppTargetFramework` property to a different Target Framework Moniker value,
for example:

```
dotnet msbuild msbuild/ice.proj /p:"AppTargetFramework=net7.0"
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

`allTests.py` executes by default the tests for .NET 6.0. If you want to run
the test with a different .NET Framework you must use `--framework` option.

For example, to run .NET 7.0 tests:

```
python allTests.py --framework=net7.0
```

or to run .NET Framework 4.5 tests on Windows:

```
python allTests.py --framework=net45
```

## NuGet Package

### Creating NuGet Packages on Windows

To create a NuGet package, open a Visual Studio command prompt and run the
following command:
```
msbuild msbuild\ice.proj /t:NuGetPack
```

This creates the `zeroc.ice.net` Nuget package in the `msbuild\zeroc.ice.net`
directory.

### Creating NuGet Packages on Linux or macOS

To create a NuGet package, open a command prompt and run the following command:

```
dotnet msbuild msbuild/ice.proj /t:NuGetPack
```

This creates the `zeroc.ice.net` Nuget package in the `msbuild/zeroc.ice.net`
directory.

## Building Ice for Xamarin Test Suite

The `msbuild\ice.xamarin.test.sln` Visual Studio solution allows building
the Ice test suite as a Xamarin application that can be deployed on iOS, Android
or UWP platforms.

The Xamarin test suite uses the Ice assemblies for .NET Standard 2.0. either
from the source distribution or using the `zeroc.ice.net` NuGet package. If
using the assemblies from the source distribution, they must be built before this
application.

### Building on Windows

#### Windows Build Requirements

* Visual Studio 2022 with following workloads:
  * Mobile development with .NET
  * .NET Core cross-platform development

#### Building the Android test controller

Open a Visual Studio 2022 command prompt:

```
MSBuild msbuild\ice.proj /t:AndroidXamarinBuild
```

#### Running the Android test suite

```
set PATH=%LOCALAPPDATA%\Android\sdk\cmdline-tools\latest\bin;%PATH%
set PATH=%LOCALAPPDATA%\Android\sdk\platform-tools;%PATH%
set PATH=%LOCALAPPDATA%\Android\sdk\emulator;%PATH%

python allTests.py --android --controller-app --config Release --platform x64
```

### Building on macOS

#### macOS Build Requirements

* Visual Studio for Mac

#### Building the Android test controller

```
msbuild msbuild/ice.proj /t:AndroidXamarinBuild
```

#### Building the iOS test controller

```
msbuild msbuild/ice.proj /t:iOSXamarinBuild
```

#### Running the Android test suite

```
export PATH=~/Library/Android/sdk/cmdline-tools/latest/bin:$PATH
export PATH=~/Library/Android/sdk/platform-tools:$PATH
export PATH=~/Library/Android/sdk/emulator:$PATH

python allTests.py --android --controller-app --config Release --platform x64
```

#### Running the iOS test suite

```
python allTests.py --controller-app --config Release --platform iphonesimulator
```

[1]: https://zeroc.com/downloads/ice
[2]: https://blogs.msdn.microsoft.com/dotnet/2017/08/14/announcing-net-standard-2-0
[3]: https://doc.zeroc.com/ice/3.7/release-notes/supported-platforms-for-ice-3-7-9
[4]: https://dotnet.microsoft.com/en-us/download/dotnet/6.0
[5]: https://dotnet.microsoft.com/en-us/download/dotnet/7.0
[6]: https://docs.microsoft.com/en-us/dotnet/framework/app-domains/enhanced-strong-naming
