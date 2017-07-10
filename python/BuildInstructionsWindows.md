# Building Ice for Python on Windows

This document describes how to build and install Ice for Python from source code
on Windows. If you prefer, you can also download [binary distributions][1] for
the supported platforms.

## Python Build Requirements

### Operating Systems and Compilers

Ice for Python was extensively tested on the operating systems and compiler versions 
listed on [supported platforms][2].

## Building the Python Extension

### With Pip

You can build the Ice for Python extension from source using pip:
```
pip install <URL of Ice source distribution for Python> 
```
### With Visual Studio 2015 and MSBuild (Python 3.6 only)

You can  build an Ice for Python 3.6 extension that links with the Ice C++
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
## Configuring your Environment for Python

Modify your environment to allow Python to find the Ice extension for Python.
The interpreter must be able to locate the extension DLL as well as the Python
source files in the `python` subdirectory. This is normally accomplished by
setting the `PYTHONPATH` environment variable to contain the necessary
subdirectory. For example, if the Ice for Python extension is installed in
`C:\Ice`, you could configure your environment as follows:
```
set PYTHONPATH=C:\Ice\python;C:\Ice\python\Win32\Release
```
## Running the Python Tests

After a successful build, you can run the tests as follows:
```
python allTests.py --config=Release --platform=Win32
```
(adjust `--config` and `--platform` to match your build)

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

[1]: https://zeroc.com/distributions/ice
[2]: https://doc.zeroc.com/display/Rel/Supported+Platforms+for+Ice+3.7.0
[3]: https://github.com/zeroc-ice/ice-builder-visualstudio
