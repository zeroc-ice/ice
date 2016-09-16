# Building Ice for Ruby on Windows

This page describes how to build and install Ice for Ruby from source code.
If you prefer, you can also download [binary distributions][1] for the
supported platforms.

## Ruby Build Requirements

### Operating Systems

Ice for Ruby is expected to build and run properly on Windows, and was
extensively tested using the operating systems and Ruby versions listed for our
[supported platforms][2].

### Prerequisites

To build Ice for Ruby you must have the following:

- Ice 3.6 development kit for C++
- Ruby 2.2
- Ruby Development Kit 4.7.2
- mingw 4.7.3 (only for 32-bit builds)

The Ruby distribution for Windows uses the MinGW compiler, therefore MinGW is
the only C++ compiler supported by Ice for Ruby.

The instructions in this file make the following assumptions about your build
environment:

1. You have installed the Ice 3.6.2 distribution using the ZeroC installer. The
default installation directory is `C:\Program Files (x86)\ZeroC\Ice-3.6.3`.
2. You have installed Ruby 2.2.4 using the Windows installer. The default
installation directory is `C:\Ruby224`.
3. You have installed the Ruby Development Kit 4.7.2 in `C:\RubyDevKit-4.7.2`.
4. For 32-bit builds, you have installed mingw 4.7.3 in `C:\mingw-4.7.3`.

If you selected different installation directories, you will need to modify the
relevant path names in the steps below to match your configuration.

## Building the Ruby Extension

Open a Windows command prompt and add Ruby to your environment:

    > C:\Ruby224\bin\setrbvars.bat

Run the `devkitvars.bat` batch file from the Ruby development kit to setup your
environment:

    > C:\RubyDevKit-4.7.2\devkitvars.bat

If you are building for Windows 32-bit:

1. Add mingw 4.7.3 to your PATH, before the mingw 4.7.2 included in the Ruby
development kit:

        > set PATH=C:\mingw-4.7.3\bin;%PATH%

1. Then double-check the desired version of g++ is in your PATH:

        > g++ -dumpversion
        4.7.3

Change to the Ice for Ruby source directory:

    > cd ruby

If you have not built Ice for C++ in the `cpp` subdirectory, set the `ICE_HOME`
environment variable to point to your Ice installation. This path must contain
forward slashes (/) as directory separators, and cannot contain any space.
If your Ice installation's path contains any space, use the DOS 8-character
name as a work-around, for example:

    > set ICE_HOME=C:/PROGRA~2/Ice-3.6.3

You can use `dir /x` to get this DOS name, for example:

    > cd c:\
    > dir /x

    Directory of c:\

    ...
    03/03/2014 03:37 PM <DIR> PROGRA~1 Program Files
    07/25/2014 12:54 PM <DIR> PROGRA~2 Program Files (x86)

Then run make to build the extension:

    > make

## Installing Ice for Ruby

You can perform an automated installation with the following command:

    > make install

This process uses the `prefix` variable in `config\Make.rules` as the
installation's root directory. The subdirectory `<prefix>\ruby` is created as a
copy of the local ruby directory and contains the Ice for Ruby extension library
(`IceRuby.so`) as well as Ruby source code. Using this installation method
requires that you modify your environment as described below.

## Configuring your Environment for Ruby

The Ruby interpreter must be able to locate the Ice extension. One way to
configure the interpreter is to define the `RUBYLIB` environment variable as
follows:

    > set RUBYLIB=C:\Ice-3.6.3\ruby

This example assumes your Ice for Ruby installation is located in the `C:\Ice-3.6.3`
directory.

In addition, you must modify your PATH environment variable to include the
following directories:

    C:\Program Files (x86)\ZeroC\Ice-3.6.3\bin
    C:\Ice-3.6.3\bin

At a command prompt, you can set your PATH as shown below:

    > set PATH=C:\Program Files (x86)\ZeroC\Ice-3.6.3\bin;C:\Ice-3.6.3\bin;%PATH%

## Running the Ruby Tests

The `test` subdirectory contains Ruby implementations of the core Ice test
suite. Python is required to run the test suite.

The test suites require that the Ice for C++ tests be built in the `cpp`
subdirectory of this source distribution.

Open a command window and change to the top-level directory. At the command
prompt, execute:

    > python allTests.py

You can also run tests individually by changing to the test directory and
running this command:

    > python run.py

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

[1]: https://zeroc.com/download.html
[2]: https://doc.zeroc.com/display/Ice36/Supported+Platforms+for+Ice+3.6.3
