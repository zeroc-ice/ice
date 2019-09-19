# Building Ice for Objective-C on macOS

This page describes how to build and install Ice for Objective-C from source
code on macOS. If you prefer, you can install a [Homebrew][1] package instead.

* [Objective\-C Build Requirements](#objective-c-build-requirements)
  * [Operating Systems and Compilers](#operating-systems-and-compilers)
* [Building Ice for Objective\-C](#building-ice-for-objective-c)
  * [Build configurations and platforms](#build-configurations-and-platforms)
  * [Ice Xcode SDK](#ice-xcode-sdk)
* [Installing an Objective\-C Source Build](#installing-an-objective-c-source-build)
* [Building the Test Suite](#building-the-test-suite)
* [Running the Test Suite](#running-the-test-suite)
  * [macOS](#macos)
  * [iOS Simulator](#ios-simulator)
  * [iOS](#ios)

## Objective-C Build Requirements

### Operating Systems and Compilers

Ice for Objective-C is currently only supported on macOS and iOS, and was
extensively tested using the operating system and compiler versions listed for
our [supported platforms][2].

## Building Ice for Objective-C

The build of Ice for Objective-C requires that you first build Ice for C++ in
the `cpp` subdirectory for the same configuration(s) and platform(s).

Review the top-level [config/Make.rules](../config/Make.rules) in your build
tree and update the configuration if needed. The comments in the file provide
more information.

Change to the Ice for Objective-C source subdirectory:
```
    cd objective-c
```
Run `make` to build the Ice Objective-C libraries and test suite. Set `V=1` to
get a more detailed build output. You can build only the libraries with the
`srcs` target, or only the tests with the `tests` target. For example:
```
    make V=1 -j8 srcs
```

### Build configurations and platforms

The Objective-C source tree supports multiple build configurations and
platforms. To see the supported configurations and platforms:
```
    make print V=supported-configs
    make print V=supported-platforms
```
To build all the supported configurations and platforms:
```
    make CONFIGS=all PLATFORMS=all -j8
```
The `arc-` configurations (for [Automatic Reference Counting][3]) apply only to
the test suite; the Ice libraries don't use these configurations, and the same
Ice for Objective-C library can be used with and without ARC.

### Ice Xcode SDK

The build system supports building Xcode SDKs for Ice. These SDKs allow you to
easily develop Ice applications with Xcode. To build Xcode SDKs, use the
`xcodesdk` configurations:
```
    make CONFIGS=xcodesdk -j8 srcs     # Build the Objective-C mapping Xcode SDK
                                       # for the default platform
```
The Xcode SDKs are built into `ice/sdk`.

## Installing an Objective-C Source Build

Simply run `make install`. This will install Ice in the directory specified by
the `prefix` variable in `../config/Make.rules`.

When compiling Ice programs, you must pass the location of the
`<prefix>/include` directory to the compiler with the `-I` option, and the
location of the library directory with the `-L` option.

## Building the Test Suite

`make all` or `make tests` builds the test suite for the platform(s) and
configuration(s) you selected.

However, in order to run the test suite on `iphoneos`, you need to build the
Objective-C Controller app from Xcode:
 - Open the Objective-C Test Controller project located in the
 `objective-c/test/ios/controller` directory.
 - Build the `Objective-C Controller` or `Objective-C ARC Controller` app.

## Cleaning the source build

Running `make clean` will remove the binaries created for the default
configuration and platform.

To clean the binaries produced for a specific configuration or platform, you
need to specify the `CONFIGS` or `PLATFORMS` variable. For example,
`make CONFIGS=xcodesdk clean` will clean the Xcode SDK build.

To clean the build for all the supported configurations and platforms, run
`make CONFIGS=all PLATFORMS=all clean`.

Running `make distclean` will also clean the build for all the configurations
and platforms. In addition, it will also remove the generated files created by
the Slice compilers.

## Running the Test Suite

Python is required to run the test suite. After a successful source build, you
can run the tests as shown below. If everything worked out, you should see lots
of `ok` messages. In case of a failure, the tests abort with `failed`.

### macOS
```
    python3 allTests.py # default config and default platform
    python3 allTests --config=... --platform=... # use the specified config and platform
```
### iOS Simulator
```
    python3 allTests.py --config=xcodesdk --platform=iphonesimulator --controller-app
```
### iOS
 - Start the `Objective-C Controller` or `Objective-C ARC Controller` app on
 your iOS device, from Xcode.
 - On your Mac:
 ```
   python3 allTests.py --config=xcodesdk --platform=iphoneos
 ```
 or
 ```
   python3 allTests.py --config=arc-xcodesdk --platform=iphoneos
 ```
 All the test clients and servers run on the iOS device, not on your Mac
 computer.

[1]: https://doc.zeroc.com/ice/3.7/release-notes/using-the-macos-binary-distribution
[2]: https://doc.zeroc.com/ice/3.7/release-notes/supported-platforms-for-ice-3-7-3
[3]: https://en.wikipedia.org/wiki/Automatic_Reference_Counting
