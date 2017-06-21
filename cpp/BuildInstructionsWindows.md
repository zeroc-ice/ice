# Building Ice for C++ on Windows

This file describes how to build Ice for C++ from sources on Windows and how
to test the resulting build.

ZeroC provides [Ice binary distributions][1] for various platforms and compilers,
including Windows and Visual Studio, so building Ice from sources is usually
unnecessary.

## Build Requirements

### Operating Systems and Compilers

Ice was extensively tested using the operating systems and compiler versions
listed on [supported platforms][2].

The build requires the [Ice Builder for Visual Studio][8]; you need version
4.3.6 or greater to build Ice.

### Third-Party Libraries

Ice has dependencies on a number of third-party libraries:

 - [bzip][3] 1.0
 - [expat][4] 2.1
 - [LMDB][5] 0.9
 - [mcpp][6] 2.7.2 (with patches)

You do not need to build these packages yourself, as ZeroC supplies
[NuGet][7] packages for all of these third-party dependencies.

The Ice build system for Windows downloads and installs the NuGet command-line
executable and these NuGet packages when you build Ice for C++. The third-party
packages are installed in the ``ice/cpp/msbuild/packages`` folder.

## Building Ice for C++

Open a Visual Studio command prompt. For example, with Visual Studio 2015, you
can open one of:

- VS2015 x86 Native Tools Command Prompt
- VS2015 x64 Native Tools Command Prompt

Using the first Command Prompt produces `Win32` binaries by default, while
the second Command Promt produces `x64` binaries by default.

In the Command Prompt, change to the `cpp` subdirectory:

    cd cpp

Now you're ready to build Ice:

    msbuild msbuild\ice.proj

This builds the Ice for C++ SDK and the Ice for C++ test suite, with
Release binaries for the default platform.

Set the MSBuild `Configuration` property to `Debug` to build debug binaries
instead:

     msbuild msbuild\ice.proj /p:Configuration=Debug

The `Configuration` property may be set to `Debug` or `Release`.

Set the MSBuild `Platform` property to `Win32` or `x64` to build binaries
for a specific platform, for example:

    msbuild msbuild\ice.proj /p:Configuration=Debug /p:Platform=x64

You can also skip the build of the test suite with the `BuildDist` target:

    msbuild msbuild\ice.proj /t:BuildDist /p:Platform=x64

To build the test suite using the NuGet binary distribution use:

    msbuild msbuild\ice.proj /p:ICE_BIN_DIST=all

If you want to build all supported platforms and configurations at once, use:

    msbuild msbuild\ice.proj /p:BuildAllConfigurations=yes

You can also sign the Ice binaries with Authenticode, by setting the following
environment variables:
 - SIGN_CERTIFICATE to your Authenticode certificate
 - SIGN_PASSWORD to the certificate password

If you want to run the test suite without building the entire source base, use this
command:

    msbuild msbuild\ice.proj /p:ICE_BIN_DIST=all

The build will automatically install ZeroC's official Ice binary NuGet packages if necessary.

## Building Ice for UWP

The steps are the same as for Building Ice for C++ above, except you must also use a
`UWP` target.

To build Ice for UWP:

    msbuild msbuild\ice.proj /t:UWPBuild

To skip the building of the test suite:

    msbuild msbuild\ice.proj /t:UWPBuildDist

To build all configurations:

    msbuild msbuild\ice.proj /t:UWPBuildDist /p:BuildAllConfigurations=yes

To build the test suite using the NuGet binary distribution use:

    msbuild msbuild\ice.proj /t:UWPBuild /p:ICE_BIN_DIST=all

## NuGet packages

You can create a NuGet package with the following command:

    msbuild msbuild\ice.proj /t:NuGetPack /p:BuildAllConfigurations=yes

This creates zeroc.ice.v120\zeroc.ice.v120.nupkg`, `zeroc.ice.v140\zeroc.ice.v140.nupkg` or
`zeroc.ice.v141\zeroc.ice.v141.nupkg` depending on the compiler you are using.

To create UWP NuGet packages, use the `UWPNuGetPack` target instead:

    msbuild msbuild\ice.proj /t:UWPNuGetPack /p:BuildAllConfigurations=yes

This creates `zeroc.ice.uwp.v140\zeroc.ice.uwp.v140.nupkg`, `zeroc.ice.uwp.v140.x64\zeroc.ice.uwp.v140.x64.nupkg`
and `zeroc.ice.uwp.v140.x86\zeroc.ice.uwp.v140.x86.nupkg` for Visual Studio 2015 builds or
`zeroc.ice.uwp.v140\zeroc.ice.uwp.v140.nupkg`, `zeroc.ice.uwp.v140.x64\zeroc.ice.uwp.v140.x64.nupkg` and
`zeroc.ice.uwp.v140.x86\zeroc.ice.uwp.v140.x86.nupkg` for Visual Studio 2017 builds.

## Running the Test Suite

Python is required to run the test suite. Additionally, the Glacier2 tests
require the Python module `passlib`, which you can install with the command:

    pip install passlib

After a successful source build, you can run the tests as follows:

    python allTests.py

For the C++11 mapping you need to use the `Cpp11-Debug` or `Cpp11-Release` configuration:

    python allTests.py --config Cpp11-Debug

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

## Running the Universal Windows Platform Test Suite

Python is required to run the test suite. Additionally, the Glacier2 tests
require the Python module `passlib`, which you can install with the command:

    pip install passlib

The scripts also require Ice for Python, you can build Ice for Python from [python](../python)
folder of this source distribution or install the Python module `zeroc-ice`,  using the
following command:

    pip install zeroc-ice

You can run the testsuite from the console using python:

    python allTest.py --uwp --controller-app --platform x64 --config Release

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

[1]: https://zeroc.com/distributions/ice
[2]: https://doc.zeroc.com/display/Rel/Supported+Platforms+for+Ice+3.7.0
[3]: http://bzip.org
[4]: https://libexpat.github.io
[5]: https://symas.com/lightning-memory-mapped-database/
[6]: https://github.com/zeroc-ice/mcpp
[7]: https://www.nuget.org
[8]: https://github.com/zeroc-ice/ice-builder-visualstudio
