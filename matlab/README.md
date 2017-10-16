# Ice for MATLAB

> *This project is in the prototype stage and is changing rapidly. Do not use
in a production application.*

## Prerequisites

The build system requires Microsoft Visual Studio 2015, the [Ice Builder for
Visual Studio Extension](https://marketplace.visualstudio.com/items?itemName=ZeroCInc.IceBuilder),
and a Perl installation.

Ice for MATLAB supports MATLAB 2016a or later.

## Build Instructions

Open a Visual Studio x64 command prompt, `VS2015 x64 Native Tools Command
Prompt`. In this Command Prompt, change to the `matlab` subdirectory:
```
cd matlab
```

Now you're ready to build Ice for MATLAB:
```
msbuild msbuild\ice.proj
```

To build in debug mode instead:
```
msbuild msbuild\ice.proj /p:Configuration=Debug
```

Upon completion, the build generates the following components:

 - Ice for C++11 library, located in `cpp\bin\x64\Release`
 - slice2matlab executable, located in `cpp\bin\x64\Release`
 - ice.mexw64 MEX file, located in `matlab\lib\x64\Release`
 - Prototype and thunk files, located in `matlab\lib\x64\Release`
 - MATLAB code for core Slice files, located in `matlab\lib\generated`
 - MATLAB code for test Slice files, located in `matlab\test\**\generated`

## Using Ice for MATLAB

### Search Path

Add the following directories to your MATLAB search path:

 - `matlab\lib`
 - `matlab\lib\generated`
 - `matlab\lib\x64\Release`

### Slice Files

Use `slice2matlab` to compile your Slice files. Run `slice2matlab -h` for a
description of its command-line options. You can place the generated `*.m`
files anywhere you like, but the enclosing directory must be in your MATLAB
search path.

### Loading the Library

The Ice for MATLAB library can be loaded with this command:
```
loadlibrary('ice', @iceproto)
```

The MEX file depends on `bzip2.dll` and `ice37++11.dll` that are part of the
Ice for C++ distribution.

### Running the Tests

Several tests have been ported to MATLAB so far. You can find them in
`matlab\test\*`.

Since Ice for MATLAB only supports client functionality, you will have to build
test servers from a different language mapping.

#### Running the Tests Manually

In a Command Prompt, start a test server from your chosen server language
mapping.

In MATLAB, change to a test directory:
```
cd matlab\test\...
```

Now you can start the MATLAB test client. Assuming the server is running on
the same host, use this command:
```
client({})
```

If you started the server on a different host, use this command instead:
```
client({'--Ice.Default.Host=<addr>'})
```

Replace `<addr>` with the host name or IP address of the server host.

#### Running the Automated Tests

Assuming you've built the C++11 test servers in Debug mode on Windows, run the
`allTests.py` script like this:

```
python allTests.py --platform=x64 --cpp-config=Cpp11-Debug
```

This command will automatically start a corresponding C++11 server for each
MATLAB client. MATLAB clients are executed using a minimized MATLAB interpreter
and the test output is copied to the Command Prompt window.
