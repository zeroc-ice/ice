# Building Ice for C++ with MinGW

*MinGW is only supported for building the Ice extension for Ruby. It is not
supported for general application development. If you prefer, you can install a
[Ruby gem][1] that contains a complete Ice for Ruby run-time.*

## Build Requirements

### Compiler

Ice for C++ was tested with the following MSYS2 MinGW compiler:

- GCC 6.3.0 and 7.2.0

The easiest way to install [MSYS2][2] and the MinGW GCC compiler is to use the
`ridk` tool provided with your Ruby 2.4 installation. Make sure the Ruby 2.4
`bin` directory is in your `PATH` and run:

    ridk install

You should select `3` to install MSYS2 and the MinGW development tools.

### Third-Party Libraries

Ice has dependencies on the following third-party libraries:

 - [bzip][3] 1.0
 - [mcpp][4] 2.7.2 (with patches)

You do not need to build these packages yourself, as ZeroC supplies a [Nuget][5]
package for mcpp and the bzip library is available from the MSYS2 MinGW
repository.

The Ice build system for Windows downloads and installs Nuget and the `mcpp`
Nuget package when you build Ice for C++ with MinGW. The third-party
packages are installed in the `ice/cpp/third-party-packages` folder.

If not already installed, you should install the bzip library using the MSYS2
package manager. Run one of the following command from a command prompt:

    ridk exec pacman -S mingw-w64-x86_64-bzip2   # 64-bit library
    ridk exec pacman -S mingw-w64-i686-bzip2     # 32-bit library

## Building Ice

The Ice build system for MinGW builds only a small subset of Ice for C++, namely
the core run time libraries (Ice, IceUtil, IceDiscovery, IceSSL), slice2cpp,
slice2rb and the corresponding tests.

Edit `config\Make.rules` to establish your build configuration. The comments
in the file provide more information.

To set up your MinGW environment, you should either open the MSYS2 MinGW 32-bit
or 64-bit shell depending on the architecture you want to build for.
Alternatively, you can enable the Ruby MSYS2 development environment from a
command prompt using the following command:

    ridk enable

This will setup your environment with the appropriate MinGW compiler for your
Ruby installation.

In the MinGW shell or command prompt, change to the `cpp` subdirectory:

    cd cpp

Now you are ready to build Ice:

    make

## Installing a C++ Source Build

Simply run `make install` from a command prompt. This will install Ice in the
directory specified by the `prefix` variable in `config\Make.rules`.

## Running the Test Suite

Python is required to run the test suite. After a successful build, you can run
the tests as follows:

    > python allTests.py

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

[1]: https://doc.zeroc.com/display/Ice36/Using+the+Ruby+Distribution
[2]: http://www.msys2.org
[3]: http://bzip.org
[4]: https://github.com/zeroc-ice/mcpp
[5]: https://www.nuget.org
