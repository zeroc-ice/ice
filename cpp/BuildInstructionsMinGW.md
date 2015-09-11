# Building Ice for C++ with MinGW

*MinGW is only supported for building the Ice extension for Ruby. It is not
supported for general application development. If you prefer, you can install
a [Ruby gem][1] that contains a complete Ice for Ruby run-time.*

## Build Requirements

### Compiler

Ice for C++ was tested with the following MinGW compilers:

- [mingw 4.7.3 32-bit][2]
- mingw 4.7.2 64-bit, included in the [Ruby Development Kit][3] version 4.7.2

*mingw 4.7.2 32-bit, included in the 32-bit Ruby Development Kit version
4.7.2, contains a very severe bug that makes it unsuitable to build Ice.*

### Third-Party Libraries

Ice has dependencies on the following third-party libraries:

 - [bzip][4] 1.0
 - [mcpp][5] 2.7.2 (with patches)

You do not need to build these packages yourself, as ZeroC supplies
[Nuget][6] packages for all of these third party dependencies.

The Ice build system for Windows downloads and installs Nuget and these 
Nuget packages when you build Ice for C++ with MinGW. The third-party 
packages are installed in the ``ice/cpp/third-party-packages`` folder.

## Building Ice

The Ice build system for MinGW builds only a small subset of Ice for C++, namely
the core run time libraries (Ice, IceUtil, IceDiscovery, IceSSL), slice2cpp,
slice2rb and the corresponding tests.

In a command window, change to the `cpp` subdirectory:

    > cd cpp

Run the `devkitvars.bat` batch file from the Ruby development kit to set up your
environment. For example, if you installed the development kit in
`C:\RubyDevKit-4.7.2`, run the following:

    > C:\RubyDevKit-4.7.2\devkitvars.bat

If you are building for Windows 32-bit:

- Add mingw 4.7.3 to your PATH, before the mingw 4.7.2 included in the Ruby
development kit. For example, if you installed mingw 4.7.3 in `C:\mingw-4.7.3`,
run the following:

        > set PATH=C:\mingw-4.7.3\bin;%PATH%

- Then double-check the desired version of g++ is in your PATH:

        > g++ -dumpversion
        4.7.3

Edit `config\Make.rules` to establish your build configuration. The comments
in the file provide more information.

Now you are ready to build Ice:

    > make

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
[2]: http://sourceforge.net/projects/mingwbuilds/files/host-windows/releases/4.7.3/32-bit/threads-win32/sjlj/x32-4.7.3-release-win32-sjlj-rev1.7z/download
[3]: http://rubyinstaller.org/downloads
[4]: http://bzip.org
[5]: https://github.com/zeroc-ice/mcpp
[6]: https://www.nuget.org
