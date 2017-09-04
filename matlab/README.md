# Ice for MATLAB

> *This project is in the prototype stage and is changing rapidly. Do not use
in a production application.*

## Prerequisites

The build system currently requires Microsoft Visual Studio 2015.

Ice for MATLAB is intended to support MATLAB 2016a or later but has only been
tested with 2017a.

## Build Instructions

Open a Visual Studio command prompt. For example, with Visual Studio 2015, you
can open one of:

- VS2015 x86 Native Tools Command Prompt
- VS2015 x64 Native Tools Command Prompt

Using the first Command Prompt produces `Win32` binaries by default, while
the second Command Prompt produces `x64` binaries by default. Select the
platform that matches your MATLAB installation.

In the Command Prompt, change to the `matlab` subdirectory:

    cd matlab

Add the MATLAB `bin` directory to your PATH:

    PATH=<MATLAB installation directory>\bin;%PATH%

Now you're ready to build Ice for MATLAB:

    msbuild msbuild\ice.proj

Upon completion, an x64 build generates the following components:

 - Ice for C++11 library, located in `cpp\bin\x64\Release`
 - slice2matlab executable, located in `cpp\bin\x64\Release`
 - icematlab.mexw64 MEX file, located in `matlab\src\IceMatlab`
 - MATLAB code for core Slice files, located in `matlab\lib\generated`

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

    loadlibrary icematlab

The MEX file depends on `bzip2.dll` and `ice37++11.dll`. The build copied
these DLLs to `matlab\src\IceMatlab`.

### Running the Test

One test has been ported to MATLAB so far, and the code is located in
`matlab\test\Ice\operations`. Since Ice for MATLAB only supports client
functionality, you will have to build and start a test server from a
different language mapping.

In a Command Prompt, start the test server.

In MATLAB, change to the test directory:

    cd matlab\test\Ice\operations

Now you can start the MATLAB test client. Assuming the server is running on
the same host, use this command:

    Client.start({})

If you started the server on a different host, use this command instead:

    Client.start({'--Ice.Default.Host=<addr>'})

Replace `<addr>` with the host name or IP address of the server host.
