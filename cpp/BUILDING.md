# Ice for C++ Build Instructions

This page describes how to build and install Ice for C++ from source.

ZeroC provides [Ice binary distributions] for many platforms and compilers, including Windows and Visual Studio,
so building Ice from source is usually unnecessary.

- [C++ Build Requirements](#c-build-requirements)
  - [Operating Systems and Compilers](#operating-systems-and-compilers)
  - [Third-Party Libraries](#third-party-libraries)
    - [Linux](#linux)
    - [macOS](#macos)
    - [Windows](#windows)
- [Building Ice for C++ on Linux or macOS](#building-ice-for-c-on-linux-or-macos)
  - [Build Configurations and Platforms](#build-configurations-and-platforms)
  - [C++11 Mapping](#c11-mapping)
  - [Ice Xcode SDK (macOS only)](#ice-xcode-sdk-macos-only)
- [Building Ice for C++ on Windows](#building-ice-for-c-on-windows)
  - [Build Using MSBuild](#build-using-msbuild)
  - [Build Using Visual Studio](#build-using-visual-studio)
- [Installing a C++ Source Build on Linux or macOS](#installing-a-c-source-build-on-linux-or-macos)
- [Creating a NuGet Package on Windows](#creating-a-nuget-package-on-windows)
- [Cleaning the Source Build on Linux or macOS](#cleaning-the-source-build-on-linux-or-macos)
- [Running the Tests](#running-the-tests)
  - [Linux, macOS, or Windows](#linux-macos-or-windows)
  - [iOS](#ios)
    - [iOS Simulator](#ios-simulator)
    - [iOS Device](#ios-device)

## C++ Build Requirements

### Operating Systems and Compilers

Ice was extensively tested using the operating systems and compiler versions listed on [supported platforms].

On Windows, the build requires Visual Studio 2019 or Visual Studio 2022. The built binaries are compatible with later
versions of Visual Studio but the build requires one of the versions mentioned above.

### Third-Party Libraries

Ice has dependencies on a number of third-party libraries:

- [bzip2] 1.0
- [expat] 2.1 or later
- [libedit] (Linux and macOS)
- [LMDB] 0.9 (LMDB is not required with the C++11 mapping)
- [mcpp] 2.7.2 with patches
- [OpenSSL] 1.0.0 or later (on Linux)

You do not need to build these packages from source.

#### Linux

Bzip, Expat, Libedit and OpenSSL are included with most Linux distributions.

ZeroC supplies binary packages for LMDB and mcpp for several Linux distributions that do not include them. You can
install these packages as shown below:

##### RHEL 10

```shell
sudo dnf install https://zeroc.com/download/ice/3.7/el10/ice-repo-3.7.el10.noarch.rpm
sudo dnf install lmdb-devel mcpp-devel
```

##### RHEL 9

```shell
sudo dnf install https://zeroc.com/download/ice/3.7/el9/ice-repo-3.7.el9.noarch.rpm
sudo dnf install lmdb-devel mcpp-devel
```

In addition, on Ubuntu and Debian distributions where the Ice for Bluetooth plug-in is supported, you need to install
the following packages in order to build the IceBT transport plug-in:

- [pkg-config] 0.29 or later
- [D-Bus] 1.10 or later
- [BlueZ] 5.37 or later

These packages are provided with the system and can be installed with:

```shell
sudo apt-get install pkg-config libdbus-1-dev libbluetooth-dev
```

#### macOS

bzip, expat and libedit are included with your system.

You can install LMDB and mcpp using Homebrew:

```shell
brew install lmdb mcpp
```

#### Windows

ZeroC provides NuGet packages for all these third-party dependencies.

The Ice build system for Windows downloads and installs the NuGet command-line executable and the required NuGet
packages when you build Ice for C++. The third-party packages are installed in the `ice/cpp/msbuild/packages` folder.

## Building Ice for C++ on Linux or macOS

Review the top-level [config/Make.rules](../config/Make.rules) in your build tree and update the configuration if
needed. The comments in the file provide more information.

In a command window, change to the `cpp` subdirectory:

```shell
cd cpp
```

Run `make` to build the Ice C++ libraries, services and test suite. Set `V=1` to get a more detailed build output. You
can build only the libraries and services with the `srcs` target, or only the tests with the `tests` target.

For example:

```shell
make V=1 -j8 srcs
```

To build the test suite using a binary distribution use:

```shell
make ICE_BIN_DIST=all
```

If the binary distribution you are using is not installed in a system wide location where the C++ compiler can
automatically find the header and library files, you also need to set `ICE_HOME`

```shell
make ICE_HOME=/opt/Ice-3.7.11 ICE_BIN_DIST=all
```

### Build Configurations and Platforms

The C++ source tree supports multiple build configurations and platforms. To see the supported configurations and
platforms:

```shell
make print V=supported-configs
make print V=supported-platforms
```

To build all the supported configurations and platforms:

```shell
make CONFIGS=all PLATFORMS=all -j8
```

### C++11 Mapping

The C++ source tree supports two different language mappings (C++98 and C++11). The default build uses the C++98
mapping. The C++11 mapping is a new mapping that uses new language features.

To build the C++11 mapping, use build configurations that are prefixed with `cpp11`, for example:

```shell
make CONFIGS=cpp11-shared -j8
```

### Ice Xcode SDK (macOS only)

The build system supports building Xcode SDKs for Ice. These SDKs allow you to easily develop Ice applications with
Xcode. To build Xcode SDKs, use the `xcodesdk` configurations. The [Ice Builder for Xcode] must be installed before
building the SDKs:

Build the C++98 mapping Xcode SDK:

```shell
make CONFIGS=xcodesdk -j8 srcs
```

Build the C++11 mapping Xcode SDK:

```shell
make CONFIGS=cpp11-xcodesdk -j8 srcs
```

The Xcode SDKs are built into `ice/sdk`.

## Building Ice for C++ on Windows

### Build Using MSBuild

Open a Visual Studio command prompt. For example, with Visual Studio 2022, you can open one of:

- VS2022 x86 Native Tools Command Prompt
- VS2022 x64 Native Tools Command Prompt

Using the first Command Prompt produces `Win32` binaries by default, while the second Command Prompt produces `x64`
binaries by default.

In the Command Prompt, change to the `cpp` subdirectory:

```shell
cd cpp
```

Now you're ready to build Ice:

```shell
msbuild /m msbuild\ice.proj
```

This builds the Ice for C++ SDK and the Ice for C++ test suite, with Release binaries for the default platform.

Set the MSBuild `Configuration` property to `Debug` to build debug binaries instead:

```shell
msbuild /m msbuild\ice.proj /p:Configuration=Debug
```

The `Configuration` property may be set to `Debug` or `Release`.

Set the MSBuild `Platform` property to `Win32` or `x64` to build binaries for a specific platform, for example:

```shell
msbuild /m msbuild\ice.proj /p:Configuration=Debug /p:Platform=x64
```

You can also skip the build of the test suite with the `BuildDist` target:

```shell
msbuild /m msbuild\ice.proj /t:BuildDist /p:Platform=x64
```

To build the test suite using the NuGet binary distribution use:

```shell
msbuild /m msbuild\ice.proj /p:ICE_BIN_DIST=all
```

### Build Using Visual Studio

Open the Visual Studio solution that corresponds to the Visual Studio version you are using.

- For Visual Studio 2022 use [msbuild/ice.v143.sln](./msbuild/ice.v143.sln)
- For Visual Studio 2019 use [msbuild/ice.v142.sln](./msbuild/ice.v142.sln)

Restore the solution NuGet packages using the NuGet package manager, if the automatic download of packages during build
is not enabled.

Using the configuration manager choose the platform and configuration you want to build.

The solution provide a project for each Ice component and each component can be built separately. When you build a
component its dependencies are built automatically.

The solutions organize the projects in two solution folders, C++11 and C++98, which correspond to the C++11 and C++98
mappings. If you want to build all the C++11 mapping components, build the C++11 solution folder; likewise if you want
to build all the C++98 mapping components, build the C++98 solution folder.

The test suite is built using separate Visual Studio solution [msbuild/ice.test.sln](./msbuild/ice.test.sln)

The solution provides a separate project for each test component, the `Cpp11-Release` and `Cpp11-Debug` build
configurations are setup to use the C++11 mapping in release and debug mode respectively. The `Release` and `Debug`
build configurations are setup to use the C++98 mapping in release and debug mode respectively.

The building of the test uses by default the local source build, and you must have built the Ice source with the same
platform and configuration than you are attempting to build the tests.

For example to build the `Cpp11-Release/x64` tests you must have built first the C++11 mapping using `Release/x64`.

It is also possible to build the tests using a C++ binary distribution, to do that you must set the `ICE_BIN_DIST`
environment variable to `all` before starting Visual Studio.

Then launch Visual Studio and open the desired test solution, you must now use NuGet package manager to restore the
NuGet packages, and the build will use Ice NuGet packages instead of your local source build.

## Installing a C++ Source Build on Linux or macOS

Simply run `make install`. This will install Ice in the directory specified by the `<prefix>` variable in
`../config/Make.rules`.

After installation, make sure that the `<prefix>/bin` directory is in your `PATH`.

If you choose to not embed a `runpath` into executables at build time (see your build settings in
`../config/Make.rules`) or did not create a symbolic link from the `runpath` directory to the installation directory,
you also need to add the library directory to your `LD_LIBRARY_PATH` (Linux) or `DYLD_LIBRARY_PATH` (macOS).

On a Linux x86_64 system:

- `<prefix>/lib64` (RHEL, SLES, Amazon)
- `<prefix>/lib/x86_64-linux-gnu` (Ubuntu)

On macOS:

- `<prefix>/lib`

When compiling Ice programs, you must pass the location of the `<prefix>/include` directory to the compiler with the
`-I` option, and the location of the library directory with the `-L` option.

If building a C++11 program, you must define the `ICE_CPP11_MAPPING` macro during compilation with the `-D` option
(`c++ -DICE_CPP11_MAPPING`) and add the `++11` suffix to the library name when linking (such as `-lIce++11`).

## Creating a NuGet Package on Windows

First build all the required configurations and platforms:

```shell
msbuild msbuild\ice.proj /p:Configuration=Release /p:Platform=x64 /t:BuildDist
msbuild msbuild\ice.proj /p:Configuration=Debug /p:Platform=x64 /t:BuildDist
msbuild msbuild\ice.proj /p:Configuration=Release /p:Platform=Win32 /t:BuildDist
msbuild msbuild\ice.proj /p:Configuration=Debug /p:Platform=Win32 /t:BuildDist
```

Then you can create the NuGet package with the following command:

```shell
msbuild msbuild\ice.proj /t:NuGetPack
```

This creates `zeroc.ice.v142\zeroc.ice.v142.nupkg` or `zeroc.ice.v143\zeroc.ice.v143.nupkg`depending on the
compiler you are using.

## Cleaning the Source Build on Linux or macOS

Running `make clean` will remove the binaries created for the default configuration and platform.

To clean the binaries produced for a specific configuration or platform, you need to specify the `CONFIGS` or
`PLATFORMS` variable. For example, `make CONFIGS=cpp11-shared clean` will clean the C++11 mapping build.

To clean the build for all the supported configurations and platforms, run `make CONFIGS=all PLATFORMS=all clean`.

Running `make distclean` will also clean the build for all the configurations and platforms. In addition, it will also
remove the generated files created by the Slice compilers.

## Running the Tests

### Linux, macOS, or Windows

Python is required to run the test suite. Additionally, the Glacier2 tests require the Python module `passlib`, which
you can install with the command:

```shell
pip install passlib
```

After a successful source build, you can run the tests as follows:

```shell
python allTests.py
```

This runs the tests with the default config (C++98) and platform.

For the C++11 mapping, you need to specify a C++11 config:

- Linux/macOS (cpp11-shared config with the default platform)

```shell
python allTests.py --config=cpp11-shared
```

- Windows C++11 debug builds

```shell
python allTests.py --config Cpp11-Debug
```

- Windows C++11 release builds

```shell
python allTests.py --config Cpp11-Release
```

If everything worked out, you should see lots of `ok` messages. In case of a failure, the tests abort with `failed`.

### iOS

The test scripts require Ice for Python. You can build Ice for Python from the [python](../python) folder of this
source distribution, or install the Python module `zeroc-ice`, using the following command:

```shell
pip install zeroc-ice
```

In order to run the test suite on `iphoneos`, you need to build the C++98 Test Controller app or C++11 Test Controller
app from Xcode:

- Build the test suite with `make` for the `xcodedsk` or `cpp11-xcodesdk` configuration, and the `iphoneos` platform.
- Open the C++ Test Controller project located in the `cpp/test/ios/controller` directory.
- Build the `C++98 Test Controller` or the `C++11 Test Controller` app (it must match the configuration(s) selected
  when building the test suite).

#### iOS Simulator

- C++98 controller

```shell
python allTests.py --config=xcodesdk --platform=iphonesimulator --controller-app
```

- C++11 controller

```shell
python allTests.py --config=cpp11-xcodesdk --platform=iphonesimulator --controller-app
```

#### iOS Device

- Start the `C++98 Test Controller` or the `C++11 Test Controller` app on your iOS device, from Xcode.

- Start the C++98 controller on your Mac:

```shell
python allTests.py --config=xcodesdk --platform=iphoneos
```

- Start the C++11 controller on your Mac:

```shell
python allTests.py --config=cpp11-xcodesdk --platform=iphoneos
```

All the test clients and servers run on the iOS device, not on your Mac computer.

[BlueZ]: http://www.bluez.org
[bzip2]: https://github.com/zeroc-ice/bzip2
[D-Bus]: https://www.freedesktop.org/wiki/Software/dbus
[expat]: https://libexpat.github.io
[Ice binary distributions]: https://zeroc.com/downloads/ice
[Ice Builder for Xcode]: https://github.com/zeroc-ice/ice-builder-xcode
[libedit]: https://thrysoee.dk/editline/
[LMDB]: https://symas.com/lightning-memory-mapped-database/
[mcpp]: https://github.com/zeroc-ice/mcpp
[OpenSSL]: https://www.openssl.org/
[pkg-config]: https://www.freedesktop.org/wiki/Software/pkg-config
[supported platforms]: https://doc.zeroc.com/ice/3.7/release-notes/supported-platforms-for-ice-3-7-11
