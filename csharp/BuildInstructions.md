# Building Ice for .NET

This page describes how to build and install Ice for .NET from source code using
Visual Studio. If you prefer, you can also download [binary distributions][1]
for the supported platforms.

## Build Requirements

### Operating Systems and Compilers

Ice for .NET was extensively tested using the operating systems and compiler
versions listed for our [supported platforms][2].

The build requires the [Ice Builder for Visual Studio][3]. You must install
version 4.3.6 or greater to build Ice.

## Compiling Ice for .NET with Visual Studio

### Preparing to Build

The build system requires the `slice2cs` compiler from Ice for C++. If you have
not built Ice for C++ in this source distribution, refer to the
[C++ build instructions](../cpp/BuildInstructionsWindows.md).

### Building Ice for .NET

Open a Visual Studio command prompt and change to the `csharp` subdirectory:

    cd csharp

To build the Ice assemblies, services and tests, run

    msbuild msbuild\ice.proj

Upon completion, the Ice assemblies are placed in the `Assemblies` subdirectory.

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

If you want to build the test suite without building the entire source base, use
this command:

    msbuild msbuild\ice.proj /p:ICE_BIN_DIST=all

The build will automatically install ZeroC's official Ice binary NuGet packages
if necessary.

## Running the .NET Tests

Python is required to run the test suite. Additionally, the Glacier2 tests
require the Python module `passlib`, which you can install with the command:

    > pip install passlib

To run the tests, open a command window and change to the top-level directory.
At the command prompt, execute:

    > python allTests.py

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

## Targeting Managed Code

Ice invokes unmanaged code to implement the following features:

- Protocol compression
- Signal processing in the Ice.Application class

if you do not require these features and prefer that the Ice run time use only
managed code, you can build using the `Debug-Managed` or `Release-Managed`
configurations:

    msbuild msbuild\ice.proj /p:Configuration=Release-Managed

## NuGet packages

To create a NuGet package for the distribution, use the following command:

    msbuild msbuild\ice.proj /t:NuGetPack

This will create `zeroc.ice.net\zeroc.ice.net.nupkg`.

[1]: https://zeroc.com/distributions/ice
[2]: https://doc.zeroc.com/display/Rel/Supported+Platforms+for+Ice+3.7.0
[3]: https://github.com/zeroc-ice/ice-builder-visualstudio
[4]: https://docs.microsoft.com/en-us/dotnet/framework/app-domains/enhanced-strong-naming
