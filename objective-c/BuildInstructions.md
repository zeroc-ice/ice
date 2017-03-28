# Building Ice for Objective-C on macOS

This page describes how to build and install Ice for Objective-C from source
code on macOS. If you prefer, you can install a [Homebrew][1] package instead.

## Objective-C Build Requirements

### Operating Systems and Compilers

Ice for Objective-C is currently only supported on macOS, and was extensively
tested using the operating system and compiler versions listed for our
[supported platforms][2].

### Ice Development Kit

You will need the Ice development kit for C++, which you can install as a binary
distribution or compile from source yourself.

## Building Ice for Objective-C

Change to the Ice for Objective-C source subdirectory:

    $ cd objective-c

If you have not built Ice for C++ in the `cpp` subdirectory, set `ICE_HOME` to
the directory of your Ice for C++ installation. For example:

    $ export ICE_HOME=/opt/Ice

Edit `config/Make.rules` to establish your build configuration. The comments in
the file provide more information.

Run `make` to build the Ice Objective-C libraries.

## Installing an Objective-C Source Build

Simply run `make install`. This will install Ice in the directory specified by
the `prefix` variable in `config/Make.rules`.

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

[1]: https://doc.zeroc.com/display/Ice36/Using+the+OS+X+Binary+Distribution
[2]: https://zeroc.com/platforms_3_6_0.html
