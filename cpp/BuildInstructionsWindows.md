# Building Ice for C++ on Windows

This file describes how to build Ice for C++ from sources on Windows, and how
to test the resulting build.

ZeroC provides [Ice binary distributions][1] for various platforms and compilers,
including Windows and Visual Studio, so building Ice from sources is usually
unnecessary.

## Build Requirements

### Operating Systems and Compilers

Ice was extensively tested using the operating systems and compiler versions
listed for our [supported platforms][2].

The build requires the [Ice Builder for Visual Studio][8], you must install
version 4.3.6 or greater to build Ice.

### Third-Party Libraries

Ice has dependencies on a number of third-party libraries:

 - [bzip][3] 1.0
 - [expat][4] 2.1
 - [LMDB][5] 0.9.16
 - [mcpp][6] 2.7.2 (with patches)

You do not need to build these packages yourself, as ZeroC supplies
[Nuget][7] packages for all these third party dependencies.

The Ice build system for Windows downloads and installs the Nuget command line
executable and these Nuget packages when you build Ice for C++. The third-party
packages are installed in the ``ice/cpp/msbuild/packages`` folder.

## Building Ice for C++

Open a command prompt for example, when using Visual Studio 2015, you have
several alternatives:

- VS2015 x86 Native Tools Command Prompt
- VS2015 x64 Native Tools Command Prompt

Using the first configurations produces 32-bit binaries, while the second
configurations produce 64-bit binaries.

In the command window, change to the `cpp` subdirectory:

    cd cpp

Now you're ready to build Ice:

    Msbuild msbuild\ice.proj

This will build the Ice for C++ developer kit and the Ice for C++ test suite.

The build will create debug libraries and binaries for the default platform. If you
want to build release libraries and binaries you can do so by setting the Msbuild
`Configuration` property to `Release`:

     MSbuild msbuild\ice.proj /p:Configuration=Release

The supported values for the Configuration property are `Debug` and `Release`

If you wan to build libraries and binaries for a different platform you need to
set the MSbuild `Platform` property, Ice for C++ supports `Win32` and `x64` platforms.

For example to build release binaries and libraries for Windows `x64` platform use:

    MSbuild msbuild\ice.proj /p:Configuration=Release /p:Platform=x64

If you want to skip building the test suite you can use the `BuildDist` MSbuild
target:

    MSbuild msbuild\ice.proj /t:BuildDist /p:Configuration=Release /p:Platform=x64

It is also possible to build all supported platforms and configurations at once,
with the following command:

    MSbuild msbuild\ice.proj /p:BuildAllConfigurations=yes

If you want to authenticode sign the Ice binaries you must set SIGN_CERTIFICATE and
SIGN_PASSWORD environment variables to the authenticode certificate path and the 
certificate password respectivelly before build.

It is also possible to build the test suite using the binary Nuget packages, use:

    MSbuild msbuild\ice.proj /p:UseBinDist=yes

## Building Ice for UWP

Open a command prompt for example, when using Visual Studio 2015, you have
several alternatives:

- VS2015 x86 Native Tools Command Prompt
- VS2015 x64 Native Tools Command Prompt

Using the first configurations produces 32-bit binaries, while the second
configurations produce 64-bit binaries.

In the command window, change to the `cpp` subdirectory:

    cd cpp

Now you're ready to build Ice:

    Msbuild msbuild\ice.proj /t:UWPBuild

This will build the Ice for UWP developer kit and the Ice for UWP test suite.

The build will create debug libraries and binaries for the default platform. If you
want to build release libraries and binaries you can do so by setting the Msbuild
`Configuration` property to `Release`:

     MSbuild msbuild\ice.proj /t:UWPBuild /p:Configuration=Release

The supported values for the Configuration property are `Debug` and `Release`

If you wan to build libraries and binaries for a different platform you need to
set the MSbuild `Platform` property, Ice for UWP supports `ARM`, `Win32` and `x64`
platforms.

For example to build release binaries and libraries for Windows `x64` platform use:

    MSbuild msbuild\ice.proj /t:UWPBuild /p:Configuration=Release /p:Platform=x64

If you want to skip building the test suite you can use the `UWPBuildDist` MSbuild
target:

    MSbuild msbuild\ice.proj /t:UWPBuildDist /p:Configuration=Release /p:Platform=x64

It is also possible to build all supported platforms and configurations at once,
with the following command:

    MSbuild msbuild\ice.proj /t:UWPBuildDist /p:BuildAllConfigurations=yes

It is also possible to build the test suite using the binary Nuget packages, use:

    MSbuild msbuild\ice.proj /t:UWPBuild /p:UseNugetBinDist=yes

## Nuget packages

To create a Nuget package for the distribution use the following command:

    MSbuild msbuild\ice.proj /t:NugetPack /p:BuildAllConfigurations=yes

This will create `zeroc.ice.v120\zeroc.ice.v120.nupkg` or `zeroc.ice.v140\zeroc.ice.v140.nupkg`
depending of the compiler version you are using to build the package.

To create UWP Nuget packages you must use the `UWPNugetPack` target instead:

    MSbuild msbuild\ice.proj /t:UWPNugetPack /p:BuildAllConfigurations=yes

This will create `zeroc.ice.uwp\zeroc.ice.uwp.nupkg`, `zeroc.ice.uwp.arm\zeroc.ice.uwp.arm.nupkg`,
`zeroc.ice.uwp.x64\zeroc.ice.uwp.x64.nupkg` and `zeroc.ice.uwp.x86\zeroc.ice.uwp.x96.nupkg` packages.

## Running the Test Suite

Python is required to run the test suite. Additionally, the Glacier2 tests
require the Python module `passlib`, which you can install with the command:

    pip install passlib

After a successful source build, you can run the tests as follows:

    python allTests.py

For C++11 mapping you need to use the `Cpp11-Debug` or `Cpp11-Release` configuration:

    $ python allTests.py --config Cpp11-Debug

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

## Running the Universal Windows Platform Test Suite

In Visual Studio 2015, open the solution file:

    cpp\msbuild\ice.testuwp.sln

Now select the configuration that matches the settings that you used to build
Ice for UWP.

To run the UWP Test suite application you can deploy the application using "Deploy
Solution" in the "Build" menu. Once deployed, you can start the application from
the Start Menu by clicking the "Ice Test Suite" icon.

In the test suite application, selecting "uwp" for the Server field allows you
to run tests with TCP and WS protocols supported by the UWP server side.

You can also use C++, C# or Java servers to run the tests, which allows you to
use  additional SSL and WSS protocols.

To use servers from C++, C# or Java language mappings, you need to build the
tests for the desired language mapping.

The test controller server is implemented in Java. Refer to the build
instructions in java subdirectory for information on building the test
controller. Use the following command to start the test controller:

    > python scripts/TestController.py

In the "Ice Test Suite" Windows Store application, select the Server language
mapping and Protocol you want to use.

[1]: https://zeroc.com/download.html
[2]: https://doc.zeroc.com/display/Ice37/Supported+Platforms+for+Ice+3.7.0
[3]: http://bzip.org
[4]: http://expat.sourceforge.net
[5]: http://symas.com/mdb/
[6]: https://github.com/zeroc-ice/mcpp
[7]: https://www.nuget.org
[8]: https://github.com/zeroc-ice/ice-builder-visualstudio
