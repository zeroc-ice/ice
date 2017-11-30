# Building Ice for .NET on Windows

This page describes how to build and install Ice for .NET from source code using
Visual Studio. If you prefer, you can also download [binary distributions][1]
for the supported platforms.

## Build Requirements

### Operating Systems and Compilers

Ice for .NET was extensively tested using the operating systems and compiler
versions listed for our [supported platforms][2].

The Visual Studio build requires the [Ice Builder for Visual Studio][3]. You must
install version 4.3.6 or greater to build Ice.

.NET Core 2.0 SDK

## Compiling Ice for .NET with Visual Studio

Open a Visual Studio command prompt and change to the `csharp` subdirectory:

```
cd csharp
```

To build the Ice assemblies, services and tests, run

```
msbuild msbuild\ice.proj
```

Upon completion, the Ice assemblies from .NET 4.5 and .NET Standard 2.0 are
placed in the `lib\net45` and `lib\netstandard2.0` directories respectively.

You can skip the build of the test suite with the `BuildDist` target:

```
msbuild msbuild\ice.proj /t:Net45Build
```

If you want to only build .NET 4.5 targets use `Net45Build` or `Net45BuildDist`
build targets:

```
msbuild msbuild\ice.proj /t:Net45Build
```

Likewise if you want to only build .NET Standard targets use `NetStandardBuild` or
`NetStandardBuildDist` targets:

```
msbuild msbuild\ice.proj /t:NetStandardBuild
```

You can add Strong Naming signatures to Ice assemblies by setting the following
environment variables:

 - PUBLIC_KEYFILE Identity public key used to delay sign the assembly
 - KEYFILE Identity full key pair used to sign the assembly

If only PUBLIC_KEYFILE is set, the assemblies are delay signed during the build
and you must re-sign the assemblies with the full identity key pair.

If only KEYFILE is set, the assemblies are fully signed during the build using
KEYFILE.

If both PUBLIC_KEYFILE and KEYFILE are set, assemblies are delay signed during
the build using PUBLIC_KEYFILE and re-signed after the build using KEYFILE.
This can be used for generating [Enhanced Strong Naming](3) signatures.

You can also sign the Ice binaries with Authenticode by setting the following
environment variables:

 - SIGN_CERTIFICATE to your Authenticode certificate
 - SIGN_PASSWORD to the certificate password

*Assembly signing is currently only supported for .NET 4.5 assemblies.*

If you want to build the test suite without building the entire source base, use
this command:

```
msbuild msbuild\ice.proj /p:ICE_BIN_DIST=all
```

The build will automatically install ZeroC's official Ice binary NuGet packages
if necessary.

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

## NuGet packages

To create a NuGet package for the distribution, use the following command:

    msbuild msbuild\ice.proj /t:NuGetPack

This will create `zeroc.ice.net\zeroc.ice.net.nupkg`.

*At this point only .NET 4.5 assemblies are included in the NuGet package*

[1]: https://zeroc.com/distributions/ice
[2]: https://doc.zeroc.com/display/Rel/Supported+Platforms+for+Ice+3.7.0
[3]: https://github.com/zeroc-ice/ice-builder-visualstudio
[4]: https://docs.microsoft.com/en-us/dotnet/framework/app-domains/enhanced-strong-naming
