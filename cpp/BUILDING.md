# Building Ice for C++

This file describes how to build Ice for C++ from source and how to test the resulting build.

ZeroC provides [Ice binary distributions][1] for many platforms and compilers, including Windows and Visual Studio, so
building Ice from source is usually unnecessary.

- [Building Ice for C++](#building-ice-for-c)
  - [C++ Build Requirements](#c-build-requirements)
    - [Operating Systems and Compilers](#operating-systems-and-compilers)
    - [Third-Party Libraries](#third-party-libraries)
      - [Linux](#linux)
        - [RHEL 9](#rhel-9)
        - [Ice for Bluetooth](#ice-for-bluetooth)
      - [macOS](#macos)
      - [Windows](#windows)
  - [Building Ice for Linux or macOS](#building-ice-for-linux-or-macos)
    - [Build configurations and platforms](#build-configurations-and-platforms)
  - [Building Ice for Windows](#building-ice-for-windows)
    - [Build Using MSBuild](#build-using-msbuild)
    - [Build Using Visual Studio](#build-using-visual-studio)
  - [Installing a C++ Source Build on Linux or macOS](#installing-a-c-source-build-on-linux-or-macos)
  - [Creating a NuGet Package on Windows](#creating-a-nuget-package-on-windows)
  - [Cleaning the source build on Linux or macOS](#cleaning-the-source-build-on-linux-or-macos)
  - [Running the Test Suite](#running-the-test-suite)
    - [Linux, macOS or Windows](#linux-macos-or-windows)
      - [iOS Simulator](#ios-simulator)
      - [iOS Device](#ios-device)

## C++ Build Requirements

### Operating Systems and Compilers

Ice was extensively tested using the operating systems and compiler versions listed on [supported platforms][2].

On Windows, the build requires Visual Studio 2022.

### Third-Party Libraries

Ice has dependencies on a number of third-party libraries:

- [bzip2][3] 1.0
- [expat][4] 2.1 or later
- [libedit][12] (Linux and macOS)
- [LMDB][5] 0.9
- [mcpp][6] 2.7.2 with patches
- [OpenSSL][7] 3.0 or later on Linux

You do not need to build these packages from source.

#### Linux

Bzip, Expat, Libedit and OpenSSL are included with most Linux distributions.

ZeroC supplies binary packages for LMDB and mcpp for several Linux distributions that do not include them. You can
install these packages as shown below:

##### RHEL 9

```shell
sudo dnf install https://zeroc.com/download/ice/3.7/el9/ice-repo-3.7.el9.noarch.rpm
sudo dnf install lmdb-devel mcpp-devel
```

##### Ice for Bluetooth

In addition, on Ubuntu and Debian distributions where the Ice for Bluetooth plug-in is supported, you need to install
the following packages in order to build the IceBT transport plug-in:

- [pkg-config][9] 0.29 or later
- [D-Bus][10] 1.10 or later
- [BlueZ][11] 5.37 or later

These packages are provided with the system and can be installed with:

```shell
sudo apt-get install pkg-config libdbus-1-dev libbluetooth-dev
```

> _We have experienced problems with BlueZ versions up to and including 5.39, as
> well as 5.44 and 5.45. At this time we recommend using the daemon (`bluetoothd`)
> from BlueZ 5.43._

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

## Building Ice for Linux or macOS

Review the top-level [config/Make.rules](../config/Make.rules) in your build tree and update the configuration if
needed. The comments in the file provide more information.

In a command window, change to the `cpp` subdirectory:

```shell
cd cpp
```

Run `make` to build the Ice C++ libraries, services and test suite. Set `V=1` to get a more detailed build output. You
can build only the libraries and services with the `srcs` target, or only the tests with the `tests` target. For
example:

```shell
make V=1 -j8 srcs
```

The build system supports specifying additional preprocessor, compiler and linker options with the `CPPFLAGS`,
`CXXFLAGS` and `LDFLAGS` variables.

### Build configurations and platforms

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

## Building Ice for Windows

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

This builds the Ice for C++ SDK and the Ice for C++ test suite, with release binaries for the default platform. The
default platform depends on the Visual Studio Command Prompt you use.

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

You can also sign the Ice binaries with Authenticode, by setting the following environment variables:

- `SIGN_CERTIFICATE` to your Authenticode certificate
- `SIGN_PASSWORD` to the certificate password
- `SIGN_SHA1` the SHA1 hash of the signing certificate

### Build Using Visual Studio

Open the [msbuild/ice.sln](./msbuild/ice.sln) Visual Studio solution.

Restore the solution NuGet packages using the NuGet package manager, if the automatic download of packages during build
is not enabled.

Using the configuration manager choose the platform and configuration you want to build.

The solution provide a project for each Ice component and each component can be built separately. When you build a
component its dependencies are built automatically.

The test suite is built using separate Visual Studio solutions:

- Ice Test Suite [msbuild/ice.test.sln](./msbuild/ice.test.sln)

The solution provides a separate project for each test component. the `Release` and `Debug` build configurations are
setup build release and debug mode respectively.

The building of the test uses the local source build, and you must have previously built the Ice source with the same
platform and configuration than you are attempting to build the tests.

For example to build the `Release/x64` tests you must have built the C++ mapping using `Release/x64`.

## Installing a C++ Source Build on Linux or macOS

Simply run `make install`. This will install Ice in the directory specified by the `<prefix>` variable in
`../config/Make.rules`.

After installation, make sure that the `<prefix>/bin` directory is in your `PATH`.

If you choose to not embed a `runpath` into executables at build time (see your build settings in
`../config/Make.rules`) or did not create a symbolic link from the `runpath` directory to the installation directory,
you also need to add the library directory to your `LD_LIBRARY_PATH` (Linux) or `DYLD_LIBRARY_PATH` (macOS).

On a Linux x86_64 system:

- `<prefix>/lib64` (RHEL)
- `<prefix>/lib/x86_64-linux-gnu` (Ubuntu)

On macOS:

- `<prefix>/lib`

When compiling Ice programs, you must pass the location of the `<prefix>/include` directory to the compiler with the
`-I` option, and the location of the library directory with the `-L` option.

## Creating a NuGet Package on Windows

You can create a NuGet package with the following command:

```shell
msbuild msbuild\ice.proj /m /t:Pack /p:BuildAllConfigurations=yes
```

This creates `ZeroC.Ice.Cpp\ZeroC.Ice.Cpp.nupkg`.

You can publish the package to your local `global-packages` source with the following command:

```shell
msbuild msbuild/ice.proj /m /p:BuildAllConfigurations=yes /t:Publish
```

If you want to build a NuGet package with binaries for a single platform and configuration, you can specify the
`Platform` and `Configuration` properties instead of the `BuildAllConfigurations` property. For example, to build a
NuGet package with release binaries for the x64 platform:

```shell
msbuild msbuild/ice.proj /m /p:Configuration=Release /p:Platform=x64 /t:Pack
```

And to publish the package:

```shell
msbuild msbuild/ice.proj /m /p:Configuration=Release /p:Platform=x64 /t:Publish
```

## Cleaning the source build on Linux or macOS

Running `make clean` will remove the binaries created for the default configuration and platform.

To clean the binaries produced for a specific configuration or platform, you need to specify the `CONFIGS` or
`PLATFORMS` variable. For example, `make CONFIGS=static clean` will clean the static configuration build.

To clean the build for all the supported configurations and platforms, run `make CONFIGS=all PLATFORMS=all clean`.

Running `make distclean` will also clean the build for all the configurations and platforms. In addition, it will also
remove the generated files created by the Slice compilers.

## Running the Test Suite

### Linux, macOS or Windows

Python 3.12 is required to run the test suite. Additionally, the Glacier2 tests require the Python module `passlib`,
which you can install with the command:

```shell
python3 -m pip install passlib
```

After a successful source build, you can run the tests as follows:

```shell
python3 allTests.py
```

### iOS

The test scripts require Ice for Python. You can build Ice for Python from the [python](../python) folder of this source
distribution, or install the Python `zeroc-ice` pip package, using the following command:

```shell
python3 -m pip install zeroc-ice
```

In order to run the test suite on iOS, you need to build the C++ Test Controller app:

For iOS devices:

```shell
make CONFIGS=static PLATFORMS=iphoneos -j8
```

For iOS simulator:

```shell
make CONFIGS=static PLATFORMS=iphonesimulator -j8
```

#### iOS Simulator

- C++ controller

```shell
python3 allTests.py --config=static --platform=iphonesimulator --controller-app
```

#### iOS Device

- Start the [C++ Test Controller](cpp/test/ios/controller) app on your iOS device, from Xcode.

- Connect to the iOS C++ Test Controller using the test script:

```shell
python3 allTests.py --config=static --platform=iphoneos
```

All the test clients and servers run on the iOS device, not on your Mac computer.

[1]: https://zeroc.com/downloads/ice
[2]: https://doc.zeroc.com/ice/3.7/release-notes/supported-platforms-for-ice-3-7-10
[3]: https://github.com/zeroc-ice/bzip2
[4]: https://libexpat.github.io
[5]: https://symas.com/lightning-memory-mapped-database/
[6]: https://github.com/zeroc-ice/mcpp
[7]: https://www.openssl.org/
[9]: https://www.freedesktop.org/wiki/Software/pkg-config
[10]: https://www.freedesktop.org/wiki/Software/dbus
[11]: http://www.bluez.org
[12]: https://thrysoee.dk/editline/
