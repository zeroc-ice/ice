# Building Ice for Python on Linux and macOS

This page describes how to build and install Ice for Python from source code on
Linux and macOS. If you prefer, you can also download [binary distributions][1]
for the supported platforms.

## Python Build Requirements

### Operating Systems and Compilers

Ice for Python is expected to build and run properly on macOS and any recent
Linux distribution for x86 and x86_64, and was extensively tested using the
operating systems and compiler versions listed for our [supported platforms][2].

### Python Versions

Ice for Python supports Python versions 2.6, 2.7, and 3.5. Note however that
your Python installation must have been built with a C++ compiler that is
compatible with the one used to build Ice for C++.

### Ice Development Kit

You will need the Ice development kit for C++, which you can install as a binary
distribution or compile from source yourself.

## Building the Python Extension

Change to the Ice for Python source subdirectory:

    $ cd python

If you have not built Ice for C++ in the `cpp` subdirectory, set `ICE_HOME` to
the directory of your Ice for C++ installation. For example:

    $ export ICE_HOME=/opt/Ice

Edit `config/Make.rules`, modify the installation prefix (if necessary), and
review the comments describing the `PYTHON_VERSION` variable.

Execute `python -V` to verify that the correct Python interpreter is in your
executable search path.

Run `make` to build the extension.

Upon successful completion, run `make install`. You may need additional user
privileges to install in the directory specified by `config/Make.rules`.

## Configuring your Environment for Python

Modify your `PYTHONPATH` environment variable to include the Ice extension
for Python. For example, assuming you installed the extension in the directory
`/opt/Ice`, you would modify your environment as shown below:

    $ export PYTHONPATH=/opt/Ice/python:$PYTHONPATH

## Running the Python Tests

After a successful build, you can run the tests as follows:

    $ python allTests.py

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

[1]: https://zeroc.com/downloads/ice/3.6
[2]: https://doc.zeroc.com/rel/ice-releases/ice-3-6/ice-3-6-5-release-notes/supported-platforms-for-ice-3-6-5
