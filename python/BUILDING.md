# Building Ice for Python from Source

## Table of Contents

* [Build roadmap](#build-roadmap)
* [Source Build](#building-ice-for-python-from-source)
  * [Prerequisites](#prerequisites)
  * [Building Ice for Python](#building-ice-for-python)
  * [Configuring your environment](#configuring-your-environment)
  * [Running the tests](#running-the-tests)
* [Python Package Build](#building-the-python-packages)
  * [Creating Python packages](#creating-python-packages)

## Build roadmap

This document describes two distinct build workflows:

1. **Source Build:**
   Builds Ice for Python directly from the source tree.
   This build **requires a prior build of Ice for C++** and is typically used for development, testing, packaging for
   Linux distributions, and contributing to Ice for Python.

   ```mermaid
   flowchart LR
       c++(Ice for C++) --> python(Ice for Python)
       python --> tests(Tests)
   ```

2. **Python Package Build:**
   Builds standalone Python packages from the source tree.
   This workflow **does not require Ice for C++** and is intended for producing distributable packages (e.g., wheels)
   for PyPI or internal use.

## Source Build

This build compiles Ice for Python directly from the source tree and requires a prior build of Ice for C++.

### Prerequisites

1. **Python 3.12 or later**

2. **Ice for C++ source build**

3. **Python dependency for Glacier2 tests**
   The Glacier2 test suite requires the `passlib` package.

### Building Ice for Python

Before building Ice for Python, you must first build the Ice for C++ source distribution.
Refer to the [build instructions](../cpp/BUILDING.md) in the `cpp` subdirectory for details.

Once Ice for C++ is built, open a command prompt and navigate to the `python` subdirectory.
To build Ice for Python, run the following commands:

**On Linux and macOS:**

```shell
make
```

**On Windows:**

```shell
MSBuild msbuild\ice.proj
```

By default, the Windows build uses the Python installation located at:

* `C:\Program Files\Python312` for `x64` builds
* `C:\Program Files (x86)\Python312-32` for `Win32` builds

If your Python installation is in a different location, set the `PythonHome` MSBuild property:

```shell
MSBuild msbuild\ice.proj /p:PythonHome=C:\Python312
```

To build a debug version for use with `python_d`, set the `Configuration` property to `Debug`:

```shell
MSBuild msbuild\ice.proj /p:Configuration=Debug
```

To change the target platform, use the `Platform` property. For example, to build for `Win32` in debug mode:

```shell
MSBuild msbuild\ice.proj /p:Platform=Win32 /p:Configuration=Debug
```

> [!IMPORTANT]
> The `Platform`, `Configuration`, and `PythonHome` settings must match your C++ build and Python version.

### Configuring your environment

You need to modify your environment to allow Python to find the Ice extension. The Python interpreter must be able to
locate the IcePy extension as well as the Python source files in the `python` subdirectory. This is normally
accomplished by setting the `PYTHONPATH` environment variable to contain the necessary subdirectory.

For example on Windows, with Ice for Python built in `C:\Ice`:

```shell
set PYTHONPATH=C:\Ice\python;C:\Ice\python\x64\Release
```

For example on Linux or macOS, with Ice for Python built in `/opt/Ice`:

```shell
export PYTHONPATH=/opt/Ice/python
```

### Running the tests

To run the tests, open a command prompt and change to the `python` subdirectory. Then run:

**On Windows:**

```shell
python allTests.py --config=Release --platform=x64
```

Adjust `--config` and `--platform` to match your build.

**On Linux and macOS:**

```shell
python allTests.py
```

If everything worked out, you should see lots of `ok` messages. In case of a failure, the tests abort with `failed`.

## Python Package Build

This build process creates standalone Python packages without requiring a prior Ice for C++ build.

### Creating Python packages

1. Create and activate a Python virtual environment:

   **On Linux and macOS:**

   ```shell
   python -m venv venv
   source venv/bin/activate
   ```

   **On Windows:**

   ```shell
   python -m venv venv
   venv\Scripts\activate
   ```

2. Install the `build` package:

   ```shell
   pip install build
   ```

3. Build the Python package:

   ```shell
   python -m build
   ```

The resulting wheel and source distribution files will be placed in the `dist/` directory.
