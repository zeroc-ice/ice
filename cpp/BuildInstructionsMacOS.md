# Building Ice for C++ on macOS

This file describes the Ice source distribution, including information about
compiler requirements, third-party dependencies, and instructions for building
and testing the distribution. If you prefer, you can install a [Homebrew][1]
package instead.

## C++ Build Requirements

### Operating Systems and Compilers

Ice was extensively tested using the operating systems and compiler versions
listed for our [supported platforms][2].

### Third-Party Libraries

Ice has dependencies on a number of third-party libraries:

 - [bzip][3] 1.0
 - [Expat][4] 2.1
 - [LMDB][5] 0.9.16 (LMDB is not required with the C++11 mapping)
 - [mcpp][6] 2.7.2 (with patches)

Expat and bzip are included with your system.

You can install LMDB and mcpp using [Homebrew][7]:

    brew install lmdb mcpp

## Building Ice

From the top-level source directory, edit `config/Make.rules` to establish your
build configuration. The comments in the file provide more information. Pay
particular attention to the variables that define the locations of the third-party
libraries.

In a command window, change to the `cpp` subdirectory:

    cd cpp

Now you're ready to build Ice:

    make

This will build the Ice core libraries, services, and tests.

### Build configurations and platforms

The C++ source tree supports multiple build configurations and platforms. To
see the supported configurations and platforms:

    make print V=supported-configs
    make print V=supported-platforms

To build all the supported configurations and platforms:

    make CONFIGS=all PLATFORMS=all

### C++11 mapping

The C++ source tree supports two different language mappings (C++98 and C++11).
The default build uses the C++98 mapping. The C++11 mapping is a new mapping
that uses new language features.

To build the C++11 mapping, use build configurations that are prefixed with
`cpp11`, for example:

    make CONFIGS=cpp11-shared

### Ice Xcode SDK

The build system supports building Xcode SDKs for Ice. These SDKs allow you to
easily develop Ice applications with Xcode. To build Xcode SDKs, use the
`xcodesdk` configurations:

    make CONFIGS=xcodesdk          # Build the C++98 mapping Xcode SDK
    make CONFIGS=cpp11-xcodesdk    # Build the C++11 mapping Xcode SDK

The Xcode SDKs are built into `ice/IceSDK`.

## Installing a C++ Source Build

Simply run `make install`. This will install Ice in the directory specified by
the `prefix` variable in `../config/Make.rules`.

After installation, make sure that the `<prefix>/bin` directory is in your
`PATH`.

If you choose to not embed a `runpath` into executables at build time (see your
build settings in `../config/Make.rules`) or did not create a symbolic link from
the `runpath` directory to the installation directory, you also need to add the
library directory to your `DYLD_LIBRARY_PATH`.

When compiling Ice programs, you must pass the location of the
`<prefix>/include` directory to the compiler with the `-I` option, and the
location of the library directory with the `-L` option.

If building a C++11 program, you must define the `ICE_CPP11_MAPPING` macro
during compilation with the `-D` option (for example `clang++
-DICE_CPP11_MAPPING`) and add the `++11` suffix to the library name when linking
(such as `-lIce++11`).

The Ice Xcode SDKs are installed in `<prefix>/lib/IceSDK`.

## Running the Test Suite

Python is required to run the test suite. Additionally, the Glacier2 tests
require the Python module `passlib`, which you can install with the command:

    pip install passlib

After a successful source build, you can run the tests as follows:

    make test

This command is equivalent to:

    python allTests.py

For the C++11 mapping it also includes the `--c++11` argument:

    $ python allTests.py --c++11

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

[1]: https://doc.zeroc.com/display/Ice37/Using+the+macOS+Binary+Distribution
[2]: https://doc.zeroc.com/display/Ice37/Supported+Platforms+for+Ice+3.7.0
[3]: http://bzip.org
[4]: http://expat.sourceforge.net
[5]: http://symas.com/mdb/
[6]: https://github.com/zeroc-ice/mcpp
[7]: http://brew.sh
