# Building Ice for Python

This document describes how to build and install Ice for Python from source.
You can also download and install a [binary distribution][1].

* [Building with Pip](#building-with-pip)
* [Building with Visual Studio 2015 and MSBuild (Python 3\.7 for Windows)](#building-with-visual-studio-2015-and-msbuild-python-37-for-windows)
* [Building on Linux or macOS](#building-on-linux-or-macos)
* [Configuring your Environment for Python](#configuring-your-environment-for-python)
* [Running the Python Tests](#running-the-python-tests)

## Building with Pip

You can build the Ice for Python extension from source using `pip`:
```
pip install <URL of Ice source distribution for Python>
```

## Building with Visual Studio 2015 and MSBuild (Python 3.7 for Windows)

You can  build an Ice for Python 3.7 extension that links with the Ice C++
DLLs using Visual Studio and MSBuild.

First, open a Visual Studio 2015 command prompt:

- VS2015 x86 Native Tools Command Prompt
or
- VS2015 x64 Native Tools Command Prompt

Using the first Command Prompt produces `Win32` binaries by default, while
the second Command Prompt produces `x64` binaries by default.

In the Command Prompt, change to the `python` subdirectory:
```
cd python
```
You must build Ice for C++ from the `cpp` subdirectory. If you have not done so,
refer to the [C++ build instructions](../cpp/BuildInstructionsWindows.md).

Then build the extension:
```
msbuild msbuild\ice.proj
```
This builds the extension with `Release` binaries for the default platform. The
extension will be placed in `python\x64\Release\IcePy.pyd` for the `x64`
platform and `python\Win32\Release\IcePy.pyd` for the `Win32` platform.

If you want to build a debug version of the extension, set the MSBuild
`Configuration` property to `Debug`:
```
msbuild msbuild\ice.proj /p:Configuration=Debug
```
The debug version of the extension will be placed in
`python\x64\Debug\IcePy_d.pyd` for the `x64` platform and
`python\Win32\Debug\IcePy_d.pyd` for the `Win32` platform.

For Debug builds, a debug version of the Python interpreter must be installed
as well.

If you want to build the extension for a different platform than the Command
Prompt's default platform, you need to set the MSBuild property `Platform`. The
supported values for this property are `Win32` and `x64`.

The following command builds the `x64` platform binaries with the `Release`
configuration:
```
msbuild msbuild\ice.proj /p:Configuration=Release /p:Platform=x64
```
This command builds the `Win32` platform binaries with the `Release`
configuration:
```
msbuild msbuild\ice.proj /p:Configuration=Release /p:Platform=Win32
```
When using the MSBuild Platform property, the build platform doesn't depend
on the command prompt's default platform.

The build will use the default location for Python defined in
`python\msbuild\ice.props`. You can override it by setting the `PythonHome`
MSBuild property. For example, the following command will use the Python
installation from `C:\Python36-AMD64` instead of the default location:
```
msbuild msbuild\ice.proj /p:Configuration=Release /p:Platform=x64 /p:PythonHome=C:\Python36-AMD64
```

## Building on Linux or macOS

Ice for Python supports Python versions 2.7 and 3.7. Note however that
your Python installation must have been built with a C++ compiler that is
compatible with the compiler used to build Ice for C++.

The build of Ice for Python requires to first build Ice for C++ in the `cpp`
subdirectory.

From the top-level source directory, edit `config/Make.rules` to establish your
build configuration. The comments in the file provide more information.

Change to the Ice for Python source subdirectory:
```
cd python
```

Execute `python -V` to verify that the correct Python interpreter is in your
executable search path.

Run `make` to build the extension.

Upon successful completion, run `make install`. You may need additional user
permissions to install in the directory specified by `config/Make.rules`.

## Configuring your Environment for Python

Modify your environment to allow Python to find the Ice extension for Python.
The python interpreter must be able to locate the IcePy extension as well as
the Python source files in the `python` subdirectory. This is normally
accomplished by setting the `PYTHONPATH` environment variable to contain the
necessary subdirectory.

For example on Windows, with Ice for Python installed in `C:\Ice`:
```
set PYTHONPATH=C:\Ice\python;C:\Ice\python\Win32\Release
```

For example on Linux or macOS, with Ice for Python installed in `/opt/Ice`:
```
export PYTHONPATH=/opt/Ice/python
```

## Running the Python Tests

After a successful build, you can run the tests as follows:

Windows:
```
python allTests.py --config=Release --platform=Win32
```
(adjust `--config` and `--platform` to match your build)

Linux/macOS:
```
python allTests.py
```

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

[1]: https://zeroc.com/distributions/ice
