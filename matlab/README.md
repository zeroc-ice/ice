# Ice for MATLAB

> *This project is in the prototype stage and is changing rapidly. Do not use
in a production application.*

## Prerequisites

The build system requires Microsoft Visual Studio 2015 and the [Ice Builder for
Visual Studio Extension](https://marketplace.visualstudio.com/items?itemName=ZeroCInc.IceBuilder).

Ice for MATLAB supports MATLAB 2016a or later.

## Build Instructions

Open a Visual Studio x64 command prompt, `VS2015 x64 Native Tools Command
Prompt`. In this Command Prompt, change to the `matlab` subdirectory:
```
cd matlab
```

Add the MATLAB `bin` directory to your PATH if it's not there already:
```
PATH=<MATLAB installation directory>\bin;%PATH%
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
 - icematlab.mexw64 MEX file, located in `matlab\src\IceMatlab`
 - MATLAB code for core Slice files, located in `matlab\lib\generated`
 - MATLAB code for test Slice files, located in `matlab\test\Ice\*\generated`

## Using Ice for MATLAB

### Search Path

Add the following directories to your MATLAB search path:

 - `matlab\lib`
 - `matlab\lib\generated`
 - `matlab\src\IceMatlab`

### Slice Files

Use `slice2matlab` to compile your Slice files. Run `slice2matlab -h` for a
description of its command-line options. You can place the generated `*.m`
files anywhere you like, but the enclosing directory must be in your MATLAB
search path.

### Loading the Library

The Ice for MATLAB library can be loaded with this command:
```
loadlibrary icematlab
```

The MEX file depends on `bzip2.dll` and `ice37++11.dll`. The build copied
these DLLs to `matlab\src\IceMatlab`.

### Running the Tests

Several tests have been ported to MATLAB so far. You can find them in
`matlab\test\Ice\*`.

Since Ice for MATLAB only supports client functionality, you will have to build
test servers from a different language mapping.

#### Running the Tests Manually

In a Command Prompt, start a test server from your chosen server language
mapping.

In MATLAB, change to a test directory:
```
cd matlab\test\Ice\...
```

Now you can start the MATLAB test client. Assuming the server is running on
the same host, use this command:
```
Client.start({})
```

If you started the server on a different host, use this command instead:
```
Client.start({'--Ice.Default.Host=<addr>'})
```

Replace `<addr>` with the host name or IP address of the server host.

#### Running the Automated Tests

Start the test controller on your server host (which may be the same host as
the MATLAB clients). For example, let's assume you want to use the C++11
servers on Windows in Debug mode:
```
python scripts\Controller.py --id=server --cpp-config=Debug
```

On the client host, change to the top-level `matlab` directory and run the
`allTests.py` script:
```
cd matlab
python allTests.py --server=server --cross=cpp
```

Note that the Python script requires an installation of Ice for Python. The
script coordinates the launching of the appropriate test server by communicating
with the test controller. Each MATLAB client is executed using a minimized
MATLAB interepreter and the test output is copied to the Command Prompt window.
