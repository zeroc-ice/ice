# Building Ice for Ruby on Windows

This page describes how to build and install Ice for Ruby from source code.
If you prefer, you can also download [binary distributions][1] for the
supported platforms.

## Ruby Build Requirements

To build Ice for Ruby on Windows, you first need to install Ruby 2.4. Older
versions are not supported.

These instructions assume you have added the Ruby 2.4 `bin` directory to your
`PATH` environment variable.

The Ruby distribution for Windows uses the [MSYS2][3] MinGW compiler, therefore
MinGW is the only C++ compiler supported by Ice for Ruby. To install the MSYS2
build environment, you can use the `ridk` tool provided with Ruby, from a
command line prompt run:
```
ridk install
```

Select `3` to install MSYS2 and the MinGW development tools.

## Building the Ruby Extension

If you have not built Ice for C++ in the `cpp` subdirectory, you should first
build it. Please refer to
[BuildInstructionsMinGW.md](../cpp/BuildInstructionsMinGW.md) for instructions.

Open a Windows command prompt and enable the Ruby MSYS2 development environment:
```
ridk enable
```

Change to the Ice for Ruby source directory:
```
cd ruby
```

Then run make to build the extension:
```
make
```

## Installing Ice for Ruby

You can perform an automated installation with the following command:
```
make install
```

This process uses the `prefix` variable in `config\Make.rules` as the
installation's root directory. The subdirectory `<prefix>\ruby` is created as a
copy of the local ruby directory and contains the Ice for Ruby extension library
(`IceRuby.so`) as well as Ruby source code. Using this installation method
requires that you modify your environment as described below.

## Configuring your Environment for Ruby

The Ruby interpreter must be able to locate the Ice extension. One way to
configure the interpreter is to define the `RUBYLIB` environment variable as
follows:
```            
set RUBYLIB=C:\Ice-3.6.5\ruby
```

This example assumes your Ice for Ruby installation is located in the
`C:\Ice-3.6.5` directory.

In addition, you must set the `RUBY_DLL_PATH` environment variable to include
the following directory:
```
C:\Ice-3.6.5\bin       (32-bit)
C:\Ice-3.6.5\bin\x64   (64-bit)
```

## Running the Ruby Tests

The `test` subdirectory contains Ruby implementations of the core Ice test
suite. Python is required to run the test suite.

The test suites require that the Ice for C++ tests be built in the `cpp`
subdirectory of this source distribution.

Open a command window and change to the top-level directory. At the command
prompt, execute:
```
python allTests.py
```

You can also run tests individually by changing to the test directory and
running this command:
```
python run.py
```

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

[1]: https://zeroc.com/downloads/ice
[2]: https://doc.zeroc.com/display/Ice36/Supported+Platforms+for+Ice+and+Ice+Touch+3.6.5
[3]: http://www.msys2.org
