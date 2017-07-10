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

## Building the Python Extension

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
privileges to install in the directory specified by `config/Make.rules`.

## Configuring your Environment for Python

Modify your `PYTHONPATH` environment variable to include the Ice extension
for Python. For example, assuming you installed the extension in the directory
`/opt/Ice`, you would modify your environment as shown below:
```
    export PYTHONPATH=/opt/Ice/python:$PYTHONPATH
```
## Running the Python Tests

After a successful build, you can run the tests as follows:
```
    python allTests.py
```
If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

[1]: https://zeroc.com/distributions/ice
[2]: https://doc.zeroc.com/display/Rel/Supported+Platforms+for+Ice+3.7.0
