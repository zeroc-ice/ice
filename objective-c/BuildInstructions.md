# Building Ice for Objective-C on macOS

This page describes how to build and install Ice for Objective-C from source
code on macOS. If you prefer, you can install a [Homebrew][1] package instead.

## Objective-C Build Requirements

### Operating Systems and Compilers

Ice for Objective-C is currently only supported on macOS and iOS, and was
extensively tested using the operating system and compiler versions listed for
our [supported platforms][2].

## Building Ice for Objective-C

The build of Ice for Objective-C requires that you first build Ice for C++ in the
`cpp` subdirectory.

From the top-level source directory, edit `config/Make.rules` to establish your
build configuration. The comments in the file provide more information.

Change to the Ice for Objective-C source subdirectory:

    $ cd objective-c

Run `make` to build the Ice Objective-C libraries.

### Build configurations and platforms

The Objective-C source tree supports multiple build configurations and platforms.
To see the supported configurations and platforms:

    make print V=supported-configs
    make print V=supported-platforms

To build all the supported configurations and platforms:

    make CONFIGS=all PLATFORMS=all

### Ice Xcode SDK

The build system supports building Xcode SDKs for Ice. These SDKs allow you to
easily develop Ice applications with Xcode. To build Xcode SDKs, use the
`xcodesdk` configurations:

    make CONFIGS=xcodesdk          # Build the Objective-C mapping Xcode SDK

The Xcode SDKs are built into `ice/IceSDK`.

## Installing an Objective-C Source Build

Simply run `make install`. This will install Ice in the directory specified by
the `prefix` variable in `../config/Make.rules`.

When compiling Ice programs, you must pass the location of the
`<prefix>/include` directory to the compiler with the `-I` option, and the
location of the library directory with the `-L` option.

## Running the Test Suite

Python is required to run the test suite. After a successful source build, you
can run the tests as follows:

    $ make test

This command is equivalent to:

    $ python allTests.py

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

[1]: https://doc.zeroc.com/display/Ice37/Using+the+macOS+Binary+Distribution
[2]: https://doc.zeroc.com/display/Ice37/Supported+Platforms+for+Ice+3.7.0
