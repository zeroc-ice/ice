# Building Ice for Python on Windows

This document describes how to build and install Ice for Python from source code
on Windows. If you prefer, you can also download [binary distributions][1] for
the supported platforms.

## Python Build Requirements

### Operating Systems and Compilers

Ice for Python is expected to build and run properly on Windows and was
extensively tested using the operating systems and compiler versions listed for
our [supported platforms][2].

The build requires the [Ice Builder for Visual Studio][8], you must install
version 4.2.0 or greater to build Ice.

### Python Versions

Ice for Python supports Python versions 2.6, 2.7, 3.3, 3.4 or 3.5. Note however
that your Python installation must have been built with a C++ compiler that is
compatible with the one used to build Ice for C++.

## Building the Python Extension

The Python interpreter is readily available on Windows platforms. You can build
it yourself using Microsoft Visual C++, or obtain a binary distribution from the
Python web site. The Python 3.5.x binary distribution is compiled with Visual
C++ 14, Python 3.4.x and Python 2.7.x are compiled with Visual C++ 10, you
should compile the Ice extension with the same Visual C++ version that your
Python binary distribution was compiled.

Using the first configurations produces 32-bit binaries, while the second
configurations produce 64-bit binaries.

Change to the Ice for Python source subdirectory:

    > cd python

You must built Ice for C++ from the `cpp` subdirectory, if you have not done so
review cpp\BuildInstructionsWindows.md first.

Building the extension:

    > MSbuild msbuild\ice.proj

This will build the extension in `Release` configuration and using the command
prompt default platform, for `x64` platform the extension will be placed in
`python\x64\Release\IcePy.pyd` and for `Win32` platform the extension will be
paced in `python\Win32\Release\IcePy.pyd`.

If you want to build a debug version of the extension you can to so by setting
the MSBuild `Configuration` property to `Debug`:

    > MSbuild msbuild\ice.proj /p:Configuration=Debug

The debug version of the extension for `x64` platform will be placed in
`python\x64\Debug\IcePy_d.pyd` and for `Win32` platform it will be placed in
`python\Win32\Debug\IcePy_d.pyd`.

> *For Debug builds a debug version of the Python interpreter must be installed.*

The supported values for the `Configuration` property are `Debug` and `Release`.

If you wan to build the extension for other platform that the command prompt
default platform, you need to set the MSbuild `Platform` property, the supported
values for this property are `Win32` and `x64`.

The following command will build the extension `x64` platform binaries with
`Release` configuration:

  > MSbuild msbuild\ice.proj /p:Configuration=Release /p:Platform=x64

And the next command will build the extension `Win32` platform binaries with
`Release` configuration:

  > MSbuild msbuild\ice.proj /p:Configuration=Release /p:Platform=Win32

> *When using the MSBuild Platform property the build platform doesn't depend on
the command prompt default platform*

The build will use a default Python location defined in
`python\msbuild\ice.props`, it can be override by setting the `PythonHome`
MSBuild property.

The following command will use Python installation from `C:\Python35-AMD64`
instead of the default location:

    > MSbuild msbuild\ice.proj /p:Configuration=Release /p:Platform=x64 /p:PythonHome=C:\Python35-AMD64

## Configuring your Environment for Python

Modify your environment to allow Python to find the Ice extension for Python.
The interpreter must be able to locate the extension DLL as well as the Python
source files in the `python` subdirectory. This is normally accomplished by
setting the `PYTHONPATH` environment variable to contain the necessary
subdirectory. For example, if the Ice for Python extension is installed in
`C:\Ice`, you could configure your environment as follows:

    > set PYTHONPATH=C:\Ice\python;C:\Ice\python\Win32\Release


## Running the Python Tests

After a successful build, you can run the tests as follows:

    $ python allTests.py --mode=Release --x86

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

[1]: https://zeroc.com/download.html
[2]: https://doc.zeroc.com/display/Ice37/Supported+Platforms+for+Ice+3.7.0
