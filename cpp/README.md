# Building Ice for C++

This file describes how to build Ice for C++ from source and how to test the
resulting build.

ZeroC provides [Ice binary distributions][1] for many platforms and compilers,
including Windows and Visual Studio, so building Ice from source is usually
unnecessary.

- [C++ Build Requirements](#c---build-requirements)
  * [Operating Systems and Compilers](#operating-systems-and-compilers)
  * [Third-Party Libraries](#third-party-libraries)
    + [AIX](#aix)
    + [Linux](#linux)
    + [macOS](#macos)
    + [Windows](#windows)
- [Building Ice for AIX, Linux or macOS](#building-ice-for-aix--linux-or-macos)
  * [Build configurations and platforms](#build-configurations-and-platforms)
  * [C++11 mapping](#c--11-mapping)
  * [Ice Xcode SDK (macOS only)](#ice-xcode-sdk--macos-only-)
- [Building Ice for Windows](#building-ice-for-windows)
  * [Build Using MSBuild](#build-using-msbuild)
  * [Build Using Visual Studio](#build-using-visual-studio)
- [Building Ice for Universal Windows (UWP)](#building-ice-for-universal-windows--uwp-)
  * [Build Using MSBuild](#build-using-msbuild-1)
  * [Build Using Visual Studio](#build-using-visual-studio-1)
- [Installing a C++ Source Build on AIX, Linux or macOS](#installing-a-c---source-build-on-aix--linux-or-macos)
- [Creating a NuGet Package on Windows](#creating-a-nuget-package-on-windows)
- [Cleaning the source build on AIX, Linux or macOS](#cleaning-the-source-build-on-aix--linux-or-macos)
- [Running the Test Suite](#running-the-test-suite)
  * [AIX, Linux, macOS or Windows](#aix--linux--macos-or-windows)
  * [iOS](#ios)
    + [iOS Simulator](#ios-simulator)
    + [iOS Device](#ios-device)
  * [Universal Windows](#universal-windows)

## C++ Build Requirements

### Operating Systems and Compilers

Ice was extensively tested using the operating systems and compiler versions
listed on [supported platforms][2].

On Windows, the build requires a recent version of Visual Studio. When building
with Visual Studio 2017 you have to install Desktop development with C++ workload
and select the optional component Windows 8.1 SDK and UCRT SDK. For UWP build you
must also install Windows 10 SDK (10.0.14393.0) available with Visual Studio 2017
installer.

### Third-Party Libraries

Ice has dependencies on a number of third-party libraries:

 - [bzip2][3] 1.0
 - [expat][4] 2.1 or later
 - [LMDB][5] 0.9 (LMDB is not required with the C++11 mapping)
 - [mcpp][6] 2.7.2 with patches
 - [OpenSSL][7] 1.0.0 or later (on AIX and Linux)

You do not need to build these packages from source.

#### AIX
OpenSSL (openssl.base) is provided by AIX.
bzip2 and bzip2-devel are included in the [IBM AIX Toolbox for Linux Applications][8].

ZeroC provide RPM packages for expat, LDMB and mcpp. You can install these packages
as shown below:
```
sudo yum install https://zeroc.com/download/ice/4.0/aix7.2/ice-repo-4.0.aix7.2.noarch.rpm
sudo yum install expat-devel lmdb-devel mcpp-devel
```

The expat-devel package contains the static library libexpat-static.a built with
xlc_r, together with header files and other development files.

#### Linux

Bzip, Expat and OpenSSL are included with most Linux distributions.

ZeroC supplies binary packages for LMDB and mcpp for several Linux distributions
that do not include them. You can install these packages as shown below:

##### Amazon Linux 2
```
sudo yum install https://zeroc.com/download/ice/4.0/amzn2/ice-repo-4.0.amzn2.noarch.rpm
sudo yum install lmdb-devel mcpp-devel
```
##### RHEL 8
```
sudo yum install https://zeroc.com/download/ice/4.0/el8/ice-repo-4.0.el8.noarch.rpm
sudo yum install lmdb-devel mcpp-devel
```
##### RHEL 7
```
sudo yum install https://zeroc.com/download/ice/4.0/el7/ice-repo-4.0.el7.noarch.rpm
sudo yum install lmdb-devel mcpp-devel
```
##### SLES 12
```
wget https://zeroc.com/download/ice/4.0/suse/zeroc-ice4.0.repo
sudo zypper addrepo zeroc-ice4.0.repo
sudo sudo rpm --import https://zeroc.com/download/GPG-KEY-zeroc-release-B6391CB2CFBA643D
sudo zypper install mcpp-devel
```

In addition, on Ubuntu and Debian distributions where the Ice for Bluetooth
plug-in is supported, you need to install the following packages in order to
build the IceBT transport plug-in:

 - [pkg-config][9] 0.29 or later
 - [D-Bus][10] 1.10 or later
 - [BlueZ][11] 5.37 or later

These packages are provided with the system and can be installed with:
```
sudo apt-get install pkg-config libdbus-1-dev libbluetooth-dev
```

> *We have experienced problems with BlueZ versions up to and including 5.39, as
well as 5.44 and 5.45. At this time we recommend using the daemon (`bluetoothd`)
from BlueZ 5.43.*

#### macOS

Expat and bzip are included with your system.

You can install LMDB and mcpp using Homebrew:
```
brew install lmdb mcpp
```

#### Windows

ZeroC provides NuGet packages for all these third-party dependencies.

The Ice build system for Windows downloads and installs the NuGet command-line
executable and the required NuGet packages when you build Ice for C++. The
third-party packages are installed in the `ice/cpp/msbuild/packages` folder.

## Building Ice for AIX, Linux or macOS

Review the top-level [config/Make.rules](../config/Make.rules) in your build
tree and update the configuration if needed. The comments in the file provide
more information.

On AIX, add /opt/freeware/bin as the first element of your PATH:
```
export PATH=/opt/freeware/bin:$PATH
```

In a command window, change to the `cpp` subdirectory:
```
cd cpp
```
Run `make` to build the Ice C++ libraries, services and test suite. Set `V=1` to
get a more detailed build output. You can build only the libraries and services
with the `srcs` target, or only the tests with the `tests` target. For example:
```
make V=1 -j8 srcs
```

The build system supports specifying additional preprocessor, compiler and
linker options with the `CPPFLAGS`, `CXXFLAGS` and `LDFLAGS` variables.

### Build configurations and platforms

The C++ source tree supports multiple build configurations and platforms. To
see the supported configurations and platforms:
```
make print V=supported-configs
make print V=supported-platforms
```
To build all the supported configurations and platforms:
```
make CONFIGS=all PLATFORMS=all -j8
```

### Ice Xcode SDK (macOS only)

The build system supports building Xcode SDKs for Ice. These SDKs allow you to
easily develop Ice applications with Xcode. To build Xcode SDKs, use the
`xcodesdk` configurations:
```
make CONFIGS=xcodesdk -j8 srcs         # Build the C++ mapping Xcode SDK
```
The Xcode SDKs are built into `ice/sdk`.

## Building Ice for Windows

### Build Using MSBuild

Open a Visual Studio command prompt. For example, with Visual Studio 2015, you
can open one of:

- VS2015 x86 Native Tools Command Prompt
- VS2015 x64 Native Tools Command Prompt

Using the first Command Prompt produces `Win32` binaries by default, while
the second Command Promt produces `x64` binaries by default.

In the Command Prompt, change to the `cpp` subdirectory:
```
cd cpp
```

Now you're ready to build Ice:
```
msbuild /m msbuild\ice.proj
```

This builds the Ice for C++ SDK and the Ice for C++ test suite, with
Release binaries for the default platform.

Set the MSBuild `Configuration` property to `Debug` to build debug binaries
instead:
```
msbuild /m msbuild\ice.proj /p:Configuration=Debug
```

The `Configuration` property may be set to `Debug` or `Release`.

Set the MSBuild `Platform` property to `Win32` or `x64` to build binaries
for a specific platform, for example:
```
msbuild /m msbuild\ice.proj /p:Configuration=Debug /p:Platform=x64
```

You can also skip the build of the test suite with the `BuildDist` target:
```
msbuild /m msbuild\ice.proj /t:BuildDist /p:Platform=x64
```

To build the test suite using the NuGet binary distribution use:
```
msbuild /m msbuild\ice.proj /p:ICE_BIN_DIST=all
```

You can also sign the Ice binaries with Authenticode, by setting the following
environment variables:

 - `SIGN_CERTIFICATE` to your Authenticode certificate
 - `SIGN_PASSWORD` to the certificate password

### Build Using Visual Studio

Open the Visual Studio solution that corresponds to the Visual Studio version
you are using.

 - For Visual Studio 2019 use [msbuild/ice.v142.sln](./msbuild/ice.v142.sln)
 - For Visual Studio 2017 use [msbuild/ice.v141.sln](./msbuild/ice.v141.sln)
 - For Visual Studio 2015 use [msbuild/ice.v140.sln](./msbuild/ice.v140.sln)
 - For Visual Studio 2013 use [msbuild/ice.v120.sln](./msbuild/ice.v120.sln)
 - For Visual Studio 2010 use [msbuild/ice.v100.sln](./msbuild/ice.v100.sln)

Restore the solution NuGet packages using the NuGet package manager, if the
automatic download of packages during build is not enabled.

Using the configuration manager choose the platform and configuration you want
to build.

The solution provide a project for each Ice component and each component can be
built separately. When you build a component its dependencies are built
automatically.

For Visual Studio 2019, Visual Studio 2017 and Visual Studio 2015, the solutions
organize the projects in two solution folders, C++11 and C++98, which correspond
to the C++11 and C++98 mappings. If you want to build all the C++11 mapping
components, build the C++11 solution folder; likewise if you want to build all
the C++98 mapping components, build the C++98 solution folder.

For Visual Studio 2013 and Visual Studio 2010. there is no separate solution
folder because only the C++98 mapping is supported with these compilers.

The test suite is built using separate Visual Studio solutions:

 - Ice Test Suite for Visual Studio 2019, Visual Studio 2017, Visual Studio 2015 and Visual Studio 2013 [msbuild/ice.test.sln](./msbuild/ice.test.sln)
 - Ice Test Suite for Visual Studio 2010 [msbuild/ice.test.v100.sln](./msbuild/ice.test.v100.sln)
 - Ice OpenSSL Test Suite for Visual Studio 2019, Visual Studio 2017, Visual Studio 2015 and Visual Studio 2013 [msbuild/ice.openssl.test.sln](./msbuild/ice.openssl.test.sln)

The solution provides a separate project for each test component, the
`Release` and `Debug` build configurations are setup to use the
C++ mapping in release and debug mode respectively.

The building of the test uses by default the local source build, and you must
have built the Ice source with the same platform and configuration than you are
attempting to build the tests.

For example to build the `Release/x64` tests you must have built first the
C++11 mapping using `Release/x64`.

It is also possible to build the tests using a C++ binary distribution, to do
that you must set the `ICE_BIN_DIST` environment variable to `all` before
starting Visual Studio.

Then launch Visual Studio and open the desired test solution, you must now use
NuGet package manager to restore the NuGet packages, and the build will use Ice
NuGet packages instead of your local source build.

## Building Ice for Universal Windows (UWP)

### Build Using MSBuild

The steps are the same as for Building Ice for Windows above, except you must
also use a `UWP` target.

To build Ice for UWP:
```
msbuild msbuild\ice.proj /t:UWPBuild
```

To skip the building of the test suite:
```
msbuild msbuild\ice.proj /t:UWPBuildDist
```

To build the test suite using the NuGet binary distribution use:
```
msbuild msbuild\ice.proj /t:UWPBuild /p:ICE_BIN_DIST=all
```

### Build Using Visual Studio

Before building Ice for UWP using Visual Studio you must build the slice2cpp
compiler from the C++98 mapping, refer to [Building Ice for
Windows](#building-ice-for-windows).

Using either Visual Studio 2017 or Visual Studio 2015, open the
[msbuild/ice.uwp.sln](./msbuild/ice.uwp.sln)

Choose the platform and configuration you want to build using the configuration
manager.

The solution provides a project for each Ice component and each component can be
built separately. When you build a component, its dependencies are built
automatically.

The test suite is built using a separate Visual Studio solution
[msbuild/ice.testuwp.sln](./msbuild/ice.testuwp.sln). This solution includes a
project for each test and a project for the UWP test controller required to run
the test suite.

It is also possible to build the tests using a C++ binary distribution, to do
that you must set `ICE_BIN_DIST` environment variable to `all` before starting
Visual Studio.

Then launch Visual Studio and open the
[msbuild/ice.testuwp.sln](./msbuild/ice.testuwp.sln) solution, you must now use
NuGet package manager to restore the NuGet packages, and the build will use Ice
NuGet packages instead of your local source build.

## Installing a C++ Source Build on AIX, Linux or macOS

Simply run `make install`. This will install Ice in the directory specified by
the `<prefix>` variable in `../config/Make.rules`.

After installation, make sure that the `<prefix>/bin` directory is in your
`PATH`.

If you choose to not embed a `runpath` into executables at build time (see your
build settings in `../config/Make.rules`) or did not create a symbolic link from
the `runpath` directory to the installation directory, you also need to add the
library directory to your `LD_LIBRARY_PATH` (AIX, Linux) or `DYLD_LIBRARY_PATH`
(macOS).

On an AIX system:
<prefix>/lib

On a Linux x86_64 system:
```
<prefix>/lib64                 (RHEL, SLES, Amazon)
<prefix>/lib/x86_64-linux-gnu  (Ubuntu)
```

On macOS:
```
<prefix>/lib
```

When compiling Ice programs, you must pass the location of the
`<prefix>/include` directory to the compiler with the `-I` option, and the
location of the library directory with the `-L` option.

If building a C++11 program, you must define the `ICE_CPP11_MAPPING` macro
during compilation with the `-D` option (`c++ -DICE_CPP11_MAPPING`) and add the
`++11` suffix to the library name when linking (such as `-lIce++11`).

## Creating a NuGet Package on Windows

You can create a NuGet package with the following command:
```
msbuild msbuild\ice.proj /t:NuGetPack /p:BuildAllConfigurations=yes
```

This creates `zeroc.ice.v100\zeroc.ice.v100.nupkg`,
`zeroc.ice.v120\zeroc.ice.v120.nupkg`, `zeroc.ice.v140\zeroc.ice.v140.nupkg`,
`zeroc.ice.v141\zeroc.ice.v141.nupkg` or `zeroc.ice.v142\zeroc.ice.v142.nupkg`
depending on the compiler you are using.

To create UWP NuGet packages, use the `UWPNuGetPack` target instead:
```
msbuild msbuild\ice.proj /t:UWPNuGetPack /p:BuildAllConfigurations=yes
```

This creates: `zeroc.ice.uwp.v140\zeroc.ice.uwp.v140.nupkg`,
`zeroc.ice.uwp.v140.x64\zeroc.ice.uwp.v140.x64.nupkg` and
`zeroc.ice.uwp.v140.x86\zeroc.ice.uwp.v140.x86.nupkg` for Visual Studio 2015
builds or `zeroc.ice.uwp.v141\zeroc.ice.uwp.v141.nupkg`,
`zeroc.ice.uwp.v141.x64\zeroc.ice.uwp.v141.x64.nupkg` and
`zeroc.ice.uwp.v141.x86\zeroc.ice.uwp.v141.x86.nupkg` for Visual Studio 2017
builds.

## Cleaning the source build on AIX, Linux or macOS

Running `make clean` will remove the binaries created for the default
configuration and platform.

To clean the binaries produced for a specific configuration or platform, you
need to specify the `CONFIGS` or `PLATFORMS` variable. For example,
`make CONFIGS=shared clean` will clean the C++ shared mapping build.

To clean the build for all the supported configurations and platforms, run
`make CONFIGS=all PLATFORMS=all clean`.

Running `make distclean` will also clean the build for all the configurations
and platforms. In addition, it will also remove the generated files created by
the Slice compilers.

## Running the Test Suite

### AIX, Linux, macOS or Windows

Python is required to run the test suite. Additionally, the Glacier2 tests
require the Python module `passlib`, which you can install with the command:
```
pip install passlib
```

After a successful source build, you can run the tests as follows:
```
python allTests.py # default config and platform
```

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

### iOS

The test scripts require Ice for Python. You can build Ice for Python from
the [python](../python) folder of this source distribution, or install the
Python module `zeroc-ice`,  using the following command:
```
pip install zeroc-ice
```

In order to run the test suite on `iphoneos`, you need to build the
C++98 Test Controller app or C++11 Test Controller app from Xcode:
 - Build the test suite with `make` for the `xcodedsk` or `cpp11-xcodesdk`
 configuration, and the `iphoneos` platform.
 - Open the C++ Test Controller project located in the
 `cpp/test/ios/controller` directory.
 - Build the `C++98 Test Controller` or the `C++11 Test Controller` app (it must
 match the configuration(s) selected when building the test suite).

#### iOS Simulator
 - C++ controller
```
python allTests.py --config=xcodesdk --platform=iphonesimulator --controller-app
```

#### iOS Device
 - Start the `C++98 Test Controller` or the `C++11 Test Controller` app on your
 iOS device, from Xcode.

 - Start the C++ controller on your Mac:
```
python allTests.py --config=xcodesdk --platform=iphoneos
```

All the test clients and servers run on the iOS device, not on your Mac
computer.

### Universal Windows

The test scripts require Ice for Python. You can build Ice for Python from
the [python](../python) folder of this source distribution, or install the
Python module `zeroc-ice`,  using the following command:
```
pip install zeroc-ice
```

You can run the testsuite from the console using python:
```
python allTests.py --uwp --controller-app --platform x64 --config Release
```

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

[1]: https://zeroc.com/downloads/ice
[2]: https://doc.zeroc.com/ice/4.0/release-notes/supported-platforms-for-ice-4-0-0
[3]: https://github.com/zeroc-ice/bzip2
[4]: https://libexpat.github.io
[5]: https://symas.com/lightning-memory-mapped-database/
[6]: https://github.com/zeroc-ice/mcpp
[7]: https://www.openssl.org/
[8]: https://www-01.ibm.com/support/docview.wss?uid=ibm10882892
[9]: https://www.freedesktop.org/wiki/Software/pkg-config
[10]: https://www.freedesktop.org/wiki/Software/dbus
[11]: http://www.bluez.org
