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

In order to build Ice for .NET from source, you need all of the following:
 - a [supported version][3] of Visual Studio
 - the [.NET Core 2.0 SDK][4], if you use Visual Studio 2017

> Note: Visual Studio 2017 version 15.3.0 or higher is required for .NET Core
> builds.

### Compiling Ice for .NET on Windows

Open a Visual Studio command prompt and change to the `csharp` subdirectory:
```
cd csharp
```

To build all Ice assemblies and the associated test suite, run:
```
msbuild msbuild\ice.proj
```

Upon completion, the Ice assemblies for the .NET Framework 4.5 and .NET Standard 2.0
are placed in the `lib\net45` and `lib\netstandard2.0` folders respectively.

> Note: the assemblies for .NET Standard 2.0 are created only when you build with
> Visual Studio 2017.

You can skip the build of the test suite with the `BuildDist` target:
```
msbuild msbuild\ice.proj /t:BuildDist
```

The `BuildNet45`, `BuildNet45Dist`, `BuildNetStandard` and `BuildNetStandardDist` targets
allow you to build assemblies only for the .NET Framework 4.5 or .NET Standard 2.0,
with or without the test suite.

The target framework for the .NET Standard tests is `netcoreapp2.0` by default. You can
target other frameworks by setting the `IceTestsTargetFrameworks` property to one or more
Target Framework Monikers (TFMs), for example:
```
msbuild msbuild\ice.proj /p:"IceTestsTargetFrameworks=net461;netcoreapp2.0"
```

This builds the test programs for `net461` and `netcoreapp2.0` (in separate folders).
The target frameworks you specify must implement .NET Standard 2.0.

#### Strong Name Signatures for .NET Framework 4.5 Assemblies

You can add Strong Naming signatures to the Ice assemblies for .NET Framework 4.5
by setting the following environment variables before building these assemblies:
 - PUBLIC_KEYFILE Identity public key used to delay sign the assembly
 - KEYFILE Identity full key pair used to sign the assembly

If only PUBLIC_KEYFILE is set, the assemblies are delay-signed during the build
and you must re-sign the assemblies later with the full identity key pair.

If only KEYFILE is set, the assemblies are fully signed during the build using
KEYFILE.

If both PUBLIC_KEYFILE and KEYFILE are set, assemblies are delay-signed during
the build using PUBLIC_KEYFILE and re-signed after the build using KEYFILE.
This can be used for generating [Enhanced Strong Naming][5] signatures.

#### Authenticode Signatures

You can sign the Ice binaries with Authenticode by setting the following
environment variables before building these assemblies:
 - SIGN_CERTIFICATE to your Authenticode certificate
 - SIGN_PASSWORD to the certificate password

*Temporary limitation: assembly signing applies only to .NET Framework 4.5 assemblies at present.*

#### Building only the Test Suite

You can build only the test suite with this command:
```
msbuild msbuild\ice.proj /p:ICE_BIN_DIST=all
```

This build retrieves and installs the `zeroc.ice.net` NuGet package if necessary.

## Building on Linux or macOS

### Linux and macOS Build Requirements

You need the [.NET Core 2.0 SDK][5] to build Ice for .NET from source.

### Compiling Ice for .NET on Linux or macOS

Open a command prompt and change to the `csharp` subdirectory:
```
cd csharp
```

Then run:
```
dotnet msbuild msbuild/ice.proj
```

Upon completion, the Ice assemblies for .NET Standard 2.0 are placed in the
`lib/netstandard2.0` directory.

You can skip the build of the test suite with the `BuildDist` target:
```
dotnet msbuild msbuild/ice.proj /t:BuildDist
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

On Windows, `allTests.py` executes by default the tests for .NET Framework 4.5.
In order to execute the tests with .NET Core framework add the `--dotnetcore` option.
For example:
```
python allTests.py --dotnetcore
```

## NuGet Package

To create a NuGet package, open a Visual Studio command prompt and run the
following command:
```
msbuild msbuild\ice.proj /t:NuGetPack
```

This creates `zeroc.ice.net\zeroc.ice.net.nupkg`.

> Note: The new NuGet package always includes assemblies for the .NET Framework 4.5.
> If you build with Visual Studio 2017, the NuGet package also includes assemblies
> for .NET Standard 2.0.

*Temporary limitation: you currently cannot create NuGet packages on Linux and macOS.*

## Building Ice for Xamarin Test Suite

The `msbuild\ice.xamarin.test.sln` Visual Studio solution allows building
the Ice test suite as a Xamarin application that can be deployed on iOS, Android
or UWP platforms.

The Xamarin test suite uses the Ice assemblies for .NET Standard 2.0. either
from the source distribution or using the zeroc.ice.net NuGet package. If using
the assembles from the source distribution, they must be built before this
application.

### Building on Windows

#### Windows Build Requirements

* Visual Studio 2017 with following workloads:
  * Universal Windows Platform development
  * Mobile development with .NET
  * .NET Core cross-platform development

#### Building the Android test controller

Open a Visual Studio 2017 command prompt:

```
MSBuild msbuild\ice.proj /t:AndroidXamarinBuild
```

#### Building the UWP test controller

Open a Visual Studio 2017 command prompt:

```
MSBuild msbuild\ice.proj /t:UWPXamarinBuild
```

#### Running the Android test suite

```
set PATH=%LOCALAPPDATA%\Android\sdk\tools;%PATH%
set PATH=%LOCALAPPDATA%\Android\sdk\tools\bin;%PATH%
set PATH=%LOCALAPPDATA%\Android\sdk\platform-tools;%PATH%
set PATH=%LOCALAPPDATA%\Android\sdk\emulator;%PATH%

cd test\xamarin\controller.Android
python allTests.py --androidemulator --controller-app --config Release --platform x64
```

#### Running the UWP test suite

```
cd test\xamarin\controller.UWP
python allTests.py --controller-app --config Release --platform x64
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
export PATH=~/Library/Android/sdk/tools/bin:$PATH
export PATH=~/Library/Android/sdk/platform-tools:$PATH
export PATH=~/Library/Android/sdk/emulator:$PATH

cd test/xamarin/controller.Android
python allTests.py --androidemulator --controller-app --config Release --platform x64
```

#### Running the iOS test suite

```
cd test/xamarin/controller.iOS
python allTests.py --controller-app --platform iphonesimulator --config Release
```

[1]: https://zeroc.com/distributions/ice
[2]: https://blogs.msdn.microsoft.com/dotnet/2017/08/14/announcing-net-standard-2-0
[3]: https://doc.zeroc.com/display/Rel/Supported+Platforms+for+Ice+3.7.1
[4]: https://www.microsoft.com/net/download
[5]: https://docs.microsoft.com/en-us/dotnet/framework/app-domains/enhanced-strong-naming
