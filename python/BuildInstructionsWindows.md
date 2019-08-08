# Building Ice for Python on Windows

This document describes how to build and install Ice for Python from source code
on Windows. If you prefer, you can also download [binary distributions][1] for
the supported platforms.

## Python Build Requirements

### Operating Systems and Compilers

Ice for Python is expected to build and run properly on Windows and was
extensively tested using the operating systems and compiler versions listed for
our [supported platforms][2].

### Python Versions

Ice for Python supports Python versions 2.6, 2.7, and 3.5. Note however that
your Python installation must have been built with a C++ compiler that is
compatible with the one used to build Ice for C++.

### Ice Development Kit

You will need the Ice development kit for C++, which you can install as a binary
distribution or compile from source yourself.

## Building the Python Extension

The Python interpreter is readily available on Windows platforms. You can build
it yourself using Microsoft Visual C++, or obtain a binary distribution from the
Python web site. The Python 3.5.x binary distribution is compiled with Visual
C++ 14, Python 3.4.x and Python 2.7.x are compiled with Visual C++ 10, you should
compile the Ice extension with the same Visual C++ version that your Python binary
distribution was compiled.

Open a command prompt that supports command-line compilation with Visual C++.
For example, you can execute the Visual C++ batch file `vcvars32.bat` to
configure your environment. Alternatively, you can start a Visual Studio Command
Prompt by selecting the appropriate entry from the Visual Studio program group
in your Start menu.

Change to the Ice for Python source subdirectory:

    > cd python

If you have not built Ice for C++ from the `cpp` subdirectory, set `ICE_HOME`
to the directory of your Ice for C++ installation. For example:

    > set ICE_HOME=C:\Ice

Edit `config\Make.rules.mak` and review the settings. In particular you must set
`CPP_COMPILER` to the appropriate compiler.

Run nmake:

    > nmake /f Makefile.mak

Upon completion, the Ice extension is created as `python\IcePy.pyd`.

> *Normally you should build with `OPTIMIZE=yes`. If you wish to build a debug
version of the Ice extension, set `OPTIMIZE=no`. In this case, you will also
need to build a debug version of the Python interpreter from sources.*

## Configuring your Environment for Python

Modify your environment to allow Python to find the Ice extension for Python.
The interpreter must be able to locate the extension DLL as well as the Python
source files in the `python` subdirectory. This is normally accomplished by
setting the `PYTHONPATH` environment variable to contain the necessary
subdirectory. For example, if the Ice for Python extension is installed in
`C:\Ice`, you could configure your environment as follows:

    > set PYTHONPATH=C:\Ice\python

## Running the Python Tests

After a successful build, you can run the tests as follows:

    $ python allTests.py

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

[1]: https://zeroc.com/downloads/ice/3.6
[2]: https://doc.zeroc.com/rel/ice-releases/ice-3-6/ice-3-6-5-release-notes/supported-platforms-for-ice-3-6-5
