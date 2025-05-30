# Building Ice for Python from Source

## Table of Contents

* [Build roadmap](#build-roadmap)
* [Building Ice for Python from source](#building-ice-for-python-from-source)
  * [Prerequisites](#prerequisites)
  * [Building Ice for Python](#building-ice-for-python)
  * [Running the tests](#running-the-tests)
  * [Generating the API reference](#generating-the-api-reference)
* [Building the Python packages](#building-the-python-packages)
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
       python -- doc --> api(API Reference)
       python --> tests(Tests)
   ```

2. **Python Package Build:**
   Builds standalone Python packages from the source tree.
   This workflow **does not require Ice for C++** and is intended for producing distributable packages (e.g., wheels)
   for PyPI or internal use.

## Building Ice for Python from source

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

### Running the tests

To run the tests, open a command prompt and change to the `python` subdirectory. Then run:

```shell
python allTests.py --all
```

### Generating the API reference

To build the API reference documentation, run the following commands from the `python/docs` subdirectory:

1. Create and activate a Python virtual environment:

   ```shell
   python -m venv venv
   source venv/bin/activate  # On macOS/Linux
   venv\Scripts\activate     # On Windows
   ```

2. Install the documentation dependencies:

   ```shell
   pip install -r requirements.txt
   ```

3. Build the documentation:

   ```shell
   make html
   ```

## Building the Python packages

This build process creates standalone Python packages without requiring a prior Ice for C++ build.

### Creating Python packages

1. Create and activate a Python virtual environment:

   ```shell
   python -m venv venv
   source venv/bin/activate  # On macOS/Linux
   venv\Scripts\activate     # On Windows
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
