# Building Ice for .NET on Windows

This page describes how to build Ice for .NET from source and package the
resulting binaries. As an alternative, you can download and install the
[zeroc.ice.net][1] NuGet package.

A source build of Ice for .NET on Windows produces two sets of assemblies:
 - assemblies for the .NET Framework 4.5
 - assemblies for [.NET Standard 2.0][2]

## Build Requirements

Ice for .NET was extensively tested using the operating systems and compiler
versions listed for our [supported platforms][3].

In order to build Ice for .NET from source, you need all of the following:
 - a supported version of Visual Studio
 - the [Ice Builder for Visual Studio][4]
 - the [.NET Core 2.0 SDK][5]

## Compiling Ice for .NET

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

You can skip the build of the test suite with the `BuildDist` target:
```
msbuild msbuild\ice.proj /t:BuildDist
```

The `Net45Build`, `Net45BuildDist`, `NetStandardBuild` and `NetStandardBuildDist` targets
allow you to build assemblies only for the .NET Framework 4.5 or .NET Standard 2.0,
with or without the test suite.

### Strong Name Signatures

You can add Strong Naming signatures to the Ice assemblies by setting the following
environment variables before building these assemblies:
 - PUBLIC_KEYFILE Identity public key used to delay sign the assembly
 - KEYFILE Identity full key pair used to sign the assembly

If only PUBLIC_KEYFILE is set, the assemblies are delay-signed during the build
and you must re-sign the assemblies later with the full identity key pair.

If only KEYFILE is set, the assemblies are fully signed during the build using
KEYFILE.

If both PUBLIC_KEYFILE and KEYFILE are set, assemblies are delay-signed during
the build using PUBLIC_KEYFILE and re-signed after the build using KEYFILE.
This can be used for generating [Enhanced Strong Naming][6] signatures.

*Temporary limitation: assembly signing applies only to .NET 4.5 assemblies at present.*

### Authenticode Signatures

You can sign the Ice binaries with Authenticode by setting the following
environment variables before building these assemblies:
 - SIGN_CERTIFICATE to your Authenticode certificate
 - SIGN_PASSWORD to the certificate password

*Temporary limitation: assembly signing applies only to .NET 4.5 assemblies at present.*

### Building only the Test Suite

You can build only the test suite with this command:
```
msbuild msbuild\ice.proj /p:ICE_BIN_DIST=all
```

This build retrieves and installs ZeroC's binary NuGet packages if necessary.

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

To run the tests with .NET Standard 2.0 instead of .NET 4.5 pass the
`--framework netcoreapp2.0` argument to `allTest.py`.
```
python allTests.py --framework netcoreapp2.0
```

## NuGet Packages

To create a NuGet package for the distribution, use the following command:
```
msbuild msbuild\ice.proj /t:NuGetPack
```

This will create `zeroc.ice.net\zeroc.ice.net.nupkg`.

*Temporary limitation: the NuGet package includes only .NET 4.5 assemblies at present.*

[1]: https://zeroc.com/distributions/ice
[2]: https://blogs.msdn.microsoft.com/dotnet/2017/08/14/announcing-net-standard-2-0
[3]: https://doc.zeroc.com/display/Rel/Supported+Platforms+for+Ice+3.7.0
[4]: https://github.com/zeroc-ice/ice-builder-visualstudio
[5]: https://www.microsoft.com/net/download/windows
[6]: https://docs.microsoft.com/en-us/dotnet/framework/app-domains/enhanced-strong-naming
