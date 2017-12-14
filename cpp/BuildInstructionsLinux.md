# Building Ice for C++ on Linux

This file describes the Ice source distribution, including information about
compiler requirements, third-party dependencies, and instructions for building
and testing the distribution. If you prefer, you can install [binary
packages][1] for supported platforms that contain pre-compiled libraries,
executables, and everything else necessary to build Ice applications on Linux.

## C++ Build Requirements

### Operating Systems and Compilers

Ice is expected to build and run properly on any recent Linux distribution for
x86 and x86_64, and was extensively tested using the operating systems and
compiler versions listed for our [supported platforms][2].

### Third-Party Packages

Ice has dependencies on a number of third-party packages. Install these packages
before building Ice for C++:

 - [bzip][3] 1.0
 - [Expat][4] 2.1
 - [LMDB][5] 0.9.16 (LMDB is not required with the C++11 mapping)
 - [mcpp][6] 2.7.2 (with patches)
 - [OpenSSL][7] 1.0.0 or later

Bzip, Expat and OpenSSL are included with most Linux distributions.

ZeroC supplies binary packages for LMDB and mcpp for several Linux distributions
that do not include them. You can install these packages as shown below:

#### Amazon Linux
```
    wget https://zeroc.com/download/GPG-KEY-zeroc-release-B6391CB2CFBA643D
    sudo rpm --import GPG-KEY-zeroc-release-B6391CB2CFBA643D
    cd /etc/yum.repos.d
    sudo wget https://dev.zeroc.com/rpm/thirdparty/zeroc-thirdparty-amzn1.repo
    sudo yum install lmdb-devel mcpp-devel
```
#### RHEL 7
```
    wget https://zeroc.com/download/GPG-KEY-zeroc-release-B6391CB2CFBA643D
    sudo rpm --import GPG-KEY-zeroc-release-B6391CB2CFBA643D
    cd /etc/yum.repos.d
    sudo wget https://dev.zeroc.com/rpm/thirdparty/zeroc-thirdparty-el7.repo
    sudo yum install lmdb-devel mcpp-devel
```
#### SLES 12
```
    wget https://zeroc.com/download/GPG-KEY-zeroc-release-B6391CB2CFBA643D
    sudo rpm --import GPG-KEY-zeroc-release-B6391CB2CFBA643D
    sudo wget https://dev.zeroc.com/rpm/thirdparty/zeroc-thirdparty-sles12.repo
    sudo zypper ar -f --repo zeroc-thirdparty-sles12.repo
    sudo zypper install mcpp-devel
```
In addition, on Ubuntu and Debian distributions where the Ice for Bluetooth
plug-in is supported, you will need to install these packages if you want to
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

## Building Ice

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

## Installing a C++ Source Build

Simply run `make install`. This will install Ice in the directory specified by
the `<prefix>` variable in `../config/Make.rules`.

After installation, make sure that the `<prefix>/bin` directory is in your
`PATH`.

If you choose to not embed a `runpath` into executables at build time (see your
build settings in `../config/Make.rules`) or did not create a symbolic link from
the `runpath` directory to the installation directory, you also need to add the
library directory to your `LD_LIBRARY_PATH`.

On an x86_64 system:

    <prefix>/lib64                 (RHEL, SLES, Amazon)
    <prefix>/lib/x86_64-linux-gnu  (Ubuntu)

When compiling Ice programs, you must pass the location of the
`<prefix>/include` directory to the compiler with the `-I` option, and the
location of the library directory with the `-L` option.

If building a C++11 program, you must define the `ICE_CPP11_MAPPING` macro
during compilation with the `-D` option (`g++ -DICE_CPP11_MAPPING`) and add the
`++11` suffix to the library name when linking (such as `-lIce++11`).

## Running the Test Suite

Python is required to run the test suite. Additionally, the Glacier2 tests
require the Python module `passlib`, which you can install with the command:
```
    pip install passlib
```
After a successful source build, you can run the tests as follows:
```
    python allTests.py # default config and platform
    python allTests.py --config=cpp11-shared # cpp11-shared config with the default platform
```

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

[1]: https://doc.zeroc.com/display/Rel/Using+the+Linux+Binary+Distributions+for+Ice+3.7.1
[2]: https://doc.zeroc.com/display/Rel/Supported+Platforms+for+Ice+3.7.1
[3]: http://bzip.org
[4]: https://libexpat.github.io
[5]: https://symas.com/lightning-memory-mapped-database/
[6]: https://github.com/zeroc-ice/mcpp
[7]: https://www.openssl.org/
[8]: https://www.freedesktop.org/wiki/Software/pkg-config
[9]: https://www.freedesktop.org/wiki/Software/dbus
[10]: http://www.bluez.org
