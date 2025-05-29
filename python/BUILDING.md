# Building Ice for Python from Source

## Table of Contents

- [Build roadmap](#build-roadmap)
- [Building Ice for Python from source](#building-ice-for-python-from-source)
  - [Prerequisites](#prerequisites)
  - [Building Ice for Python](#building-ice-for-python)
  - [Running the tests](#running-the-tests)
  - [Generating the API reference](#generating-the-api-reference)
- [Building the Python packages](#building-the-python-packages)
  - [Creating Python packages](#creating-python-packages)

## Build roadmap

This document covers two distinct build workflows:

1. **Source Build:** Builds Ice for Python directly from the source tree.
   This build **requires a prior build of Ice for C++**, and is typically used for development, testing, packaging into
   Linux distributions, and contributing to the Ice for Python source.

   ```mermaid
   flowchart LR
       c++(Ice for C++) --> python(Ice for Python)
       python -- doc --> api(API Reference)
       python --> tests(Tests)
   ```

2. **Python Package Build:** Builds standalone Python packages from the source tree.
   This build path **does not require Ice for C++** and is intended for producing distributable packages
   (e.g., wheels) for PyPI or internal use.

## Building Ice for Python from Source

This build compiles Ice for Python directly from the source tree and requires a prior build of Ice for C++.

### Prerequisites

1. **Python 3.12 or later**

2. **Ice for C++**\
   Refer to the [build instructions](../cpp/BUILDING.md) in the `cpp` folder for details on how to build the C++ source distribution.

3. **Python dependency for Glacier2 tests**\
   The Glacier2 test suite requires the `passlib` package.

### Building Ice for Python

You can build Ice for Python directly from the source distribution using the following commands:

**Linux and macOS:**

```shell
make
```

**On Windows:**

```shell
MSBuild msbuild\ice.proj
```

By default, the Windows build uses the Python installation located at:

- `C:\Program Files\Python312` for `x64` builds
- `C:\Program Files (x86)\Python312-32` for `Win32` builds

If your Python installation is in a different location, set the `PythonHome` MSBuild property to the appropriate path:

```shell
MSBuild msbuild\ice.proj /p:PythonHome=C:\Python312
```

To build a debug version of the extension for use with `python_d`, set the `Configuration` property to `Debug`:

```shell
MSBuild msbuild\ice.proj /p:Configuration=Debug
```

You can also change the target platform using the Platform property. For example, to perform a Win32 debug build:

```shell
MSBuild msbuild\ice.proj /p:Platform=Win32 /p:Configuration=Debug
```

### Running the tests

To run the tests, open a command prompt and change to the `python` subdirectory. At the command prompt, execute:

```shell
python allTests.py --all
```

If everything worked out, you should see lots of `ok` messages. In case of a failure, the tests abort with `failed`.

### Generating the API reference

To build the API reference documentation, follow these steps from the `python/docs` subdirectory:

- Create and activate a Python virtual environment:

  ```shell
  python -m venv venv
  source venv/bin/activate  # On macOS/Linux
  venv\Scripts\activate     # On Windows
  ```

- Install the documentation dependencies:

  ```shell
  pip install -r requirements.txt
  ```

- Build the documentation:

  ```shell
  make html
  ```

## Building the Python Packages

This build creates standalone Python packages without requiring a prior Ice for C++ source build.

### Creating Python packages

- Create and activate a Python virtual environment:

  ```shell
  python -m venv venv
  source venv/bin/activate  # On macOS/Linux
  venv\Scripts\activate     # On Windows
  ```

- Install the build package:

  ```shell
  pip install build
  ```

- Build the Python package:

  ```shell
  python -m build
  ```

The resulting wheel and source distribution files will be placed in the `dist/` directory.
