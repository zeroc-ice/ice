# Building Ice for C++

This file describes how to build Ice for C++ from source and how to test the
resulting build.

ZeroC provides [Ice binary distributions][1] for many platforms and compilers,
including Windows and Visual Studio, so building Ice from source is usually
unnecessary.

* [C++ Build Requirements](#c-build-requirements)
  * [Operating Systems and Compilers](#operating-systems-and-compilers)
  * [Third-Party Libraries](#third-party-libraries)
    * [Linux](#linux)
    * [macOS](#macos)
    * [Windows](#windows)
* [Building Ice for Linux or macOS](#building-ice-for-linux-or-macos)
  * [Build configurations and platforms](#build-configurations-and-platforms)
  * [C++11 mapping](#c11-mapping)
  * [Ice Xcode SDK (macOS only)](#ice-xcode-sdk-macos-only)
* [Building Ice for Windows](#building-ice-for-windows)
* [Building Ice for Universal Windows (UWP)](#building-ice-for-universal-windows-uwp)
* [Installing a C++ Source Build on Linux or macOS](#installing-a-c-source-build-on-linux-or-macos)
* [Creating a NuGet Package on Windows](#creating-a-nuget-package-on-windows)
* [Running the Test Suite](#running-the-test-suite)
  * [Linux, macOS or Windows](#linux-macos-or-windows)
  * [iOS](#ios)
  * [Universal Windows](#universal-windows)

## C++ Build Requirements

### Operating Systems and Compilers

Ice was extensively tested using the operating systems and compiler versions
listed on [supported platforms][2].

On Windows, the build requires a recent version of Visual Studio and the
[Ice Builder for Visual Studio][13].

### Third-Party Libraries

Ice has dependencies on a number of third-party libraries:

 - [bzip][3] 1.0
 - [expat][4] 2.1 or later
 - [LMDB][5] 0.9 (LMDB is not required with the C++11 mapping)
 - [mcpp][6] 2.7.2 with patches
 - [OpenSSL][7] 1.0.0 or later (only on Linux)

You do not need to build these packages from source.

#### Linux

Bzip, Expat and OpenSSL are included with most Linux distributions.

ZeroC supplies binary packages for LMDB and mcpp for several Linux distributions
that do not include them. You can install these packages as shown below:

* Amazon Linux
```
wget https://zeroc.com/download/GPG-KEY-zeroc-release-B6391CB2CFBA643D
sudo rpm --import GPG-KEY-zeroc-release-B6391CB2CFBA643D
cd /etc/yum.repos.d
sudo wget https://dev.zeroc.com/rpm/thirdparty/zeroc-thirdparty-amzn1.repo
sudo yum install lmdb-devel mcpp-devel
```
* RHEL 7
```
wget https://zeroc.com/download/GPG-KEY-zeroc-release-B6391CB2CFBA643D
sudo rpm --import GPG-KEY-zeroc-release-B6391CB2CFBA643D
cd /etc/yum.repos.d
sudo wget https://dev.zeroc.com/rpm/thirdparty/zeroc-thirdparty-el7.repo
sudo yum install lmdb-devel mcpp-devel
```
* SLES 12
```
wget https://zeroc.com/download/GPG-KEY-zeroc-release-B6391CB2CFBA643D
sudo rpm --import GPG-KEY-zeroc-release-B6391CB2CFBA643D
sudo wget https://dev.zeroc.com/rpm/thirdparty/zeroc-thirdparty-sles12.repo
sudo zypper ar -f --repo zeroc-thirdparty-sles12.repo
sudo zypper install mcpp-devel
```

In addition, on Ubuntu and Debian distributions where the Ice for Bluetooth
plug-in is supported, you need to install the following packages in order to
build the IceBT transport plug-in:

 - [pkg-config][8] 0.29 or later
 - [D-Bus][9] 1.10 or later
 - [BlueZ][10] 5.37 or later

These packages are provided with the system and can be installed with:
```
sudo apt-get install pkg-config libdbus-1-dev libbluetooth-dev
```

> *We have experienced problems with BlueZ versions up to and including 5.39, as
well as 5.44 and 5.45. At this time we recommend using the daemon (`bluetoothd`)
from BlueZ 5.43.*

#### macOS

Expat and bzip are included with your system.

You can install LMDB and mcpp using [Homebrew][11]:
```
brew install lmdb mcpp
```

#### Windows

ZeroC provides [NuGet][12] packages for all these third-party dependencies.

The Ice build system for Windows downloads and installs the NuGet command-line
executable and the required NuGet packages when you build Ice for C++. The
third-party packages are installed in the `ice/cpp/msbuild/packages` folder.

## Building Ice for Linux or macOS

Review the top-level [config/Make.rules](../config/Make.rules) in your build
tree and update the configuration if needed. The comments in the file provide
more information.

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

### C++11 mapping

The C++ source tree supports two different language mappings (C++98 and C++11).
The default build uses the C++98 mapping. The C++11 mapping is a new mapping
that uses new language features.

To build the C++11 mapping, use build configurations that are prefixed with
`cpp11`, for example:
```
make CONFIGS=cpp11-shared -j8
```
### Ice Xcode SDK (macOS only)

The build system supports building Xcode SDKs for Ice. These SDKs allow you to
easily develop Ice applications with Xcode. To build Xcode SDKs, use the
`xcodesdk` configurations:
```
make CONFIGS=xcodesdk -j8 srcs         # Build the C++98 mapping Xcode SDK
make CONFIGS=cpp11-xcodesdk -j8 srcs   # Build the C++11 mapping Xcode SDK
```
The Xcode SDKs are built into `ice/sdk`.

## Building Ice for Windows

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
msbuild msbuild\ice.proj
```

This builds the Ice for C++ SDK and the Ice for C++ test suite, with
Release binaries for the default platform.

Set the MSBuild `Configuration` property to `Debug` to build debug binaries
instead:
```
msbuild msbuild\ice.proj /p:Configuration=Debug
```

The `Configuration` property may be set to `Debug` or `Release`.

Set the MSBuild `Platform` property to `Win32` or `x64` to build binaries
for a specific platform, for example:
```
msbuild msbuild\ice.proj /p:Configuration=Debug /p:Platform=x64
```

You can also skip the build of the test suite with the `BuildDist` target:
```
msbuild msbuild\ice.proj /t:BuildDist /p:Platform=x64
```

To build the test suite using the NuGet binary distribution use:
```
msbuild msbuild\ice.proj /p:ICE_BIN_DIST=all
```

You can also sign the Ice binaries with Authenticode, by setting the following
environment variables:

 - `SIGN_CERTIFICATE` to your Authenticode certificate
 - `SIGN_PASSWORD` to the certificate password

## Building Ice for Universal Windows (UWP)

The steps are the same as for Building Ice for Windows above, except you must also
use a `UWP` target.

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

## Installing a C++ Source Build on Linux or macOS

Simply run `make install`. This will install Ice in the directory specified by
the `<prefix>` variable in `../config/Make.rules`.

After installation, make sure that the `<prefix>/bin` directory is in your
`PATH`.

If you choose to not embed a `runpath` into executables at build time (see your
build settings in `../config/Make.rules`) or did not create a symbolic link from
the `runpath` directory to the installation directory, you also need to add the
library directory to your `LD_LIBRARY_PATH` (Linux) or `DYLD_LIBRARY_PATH (macOS).

On a Linux x86_64 system:
```
<prefix>/lib64                 (RHEL, SLES, Amazon)
prefix>/lib/x86_64-linux-gnu  (Ubuntu)
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
`zeroc.ice.v120\zeroc.ice.v120.nupkg`, `zeroc.ice.v140\zeroc.ice.v140.nupkg` or
`zeroc.ice.v141\zeroc.ice.v141.nupkg`
depending on the compiler you are using.

To create UWP NuGet packages, use the `UWPNuGetPack` target instead:
```
msbuild msbuild\ice.proj /t:UWPNuGetPack /p:BuildAllConfigurations=yes
```

This creates: `zeroc.ice.uwp.v140\zeroc.ice.uwp.v140.nupkg`,
`zeroc.ice.uwp.v140.x64\zeroc.ice.uwp.v140.x64.nupkg` and
`zeroc.ice.uwp.v140.x86\zeroc.ice.uwp.v140.x86.nupkg` for Visual Studio 2015
builds or `zeroc.ice.uwp.v140\zeroc.ice.uwp.v140.nupkg`,
`zeroc.ice.uwp.v140.x64\zeroc.ice.uwp.v140.x64.nupkg` and
`zeroc.ice.uwp.v140.x86\zeroc.ice.uwp.v140.x86.nupkg` for Visual Studio 2017
builds.

## Running the Test Suite

Python is required to run the test suite. Additionally, the Glacier2 tests
require the Python module `passlib`, which you can install with the command:
```
pip install passlib
```

### Linux, macOS or Windows

After a successful source build, you can run the tests as follows:
```
python allTests.py # default config (C++98) and platform
```

For the C++11 mapping, you need to specify a C++11 config:

* Linux/macOS
```
 python allTests.py --config=cpp11-shared # cpp11-shared config with the default platform
```

* Windows
```
python allTests.py --config Cpp11-Debug
python allTests.py --config Cpp11-Release
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

* iOS Simulator
```
python allTests.py --config=xcodesdk --platform=iphonesimulator --controller-app
python allTests.py --config=cpp11-xcodesdk --platform=iphonesimulator --controller-app
```

* iOS
 - Start the `C++98 Test Controller` or the `C++11 Test Controller` app on your
 iOS device, from Xcode.
 - On your Mac:
```
python allTests.py --config=xcodesdk --platform=iphoneos # C++98 controller
python allTests.py --confg=cpp11-xcodesdk --platform=iphoneos # C++11 controller
```

All the test clients and servers run on the iOS device, not on your Mac computer.

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

[1]: https://zeroc.com/distributions/ice
[2]: https://doc.zeroc.com/display/Rel/Supported+Platforms+for+Ice+3.7.1
[3]: http://bzip.org
[4]: https://libexpat.github.io
[5]: https://symas.com/lightning-memory-mapped-database/
[6]: https://github.com/zeroc-ice/mcpp
[7]: https://www.openssl.org/
[8]: https://www.freedesktop.org/wiki/Software/pkg-config
[9]: https://www.freedesktop.org/wiki/Software/dbus
[10]: http://www.bluez.org
[11]: https://brew.sh
[12]: https://www.nuget.org
[13]: https://github.com/zeroc-ice/ice-builder-visualstudio
