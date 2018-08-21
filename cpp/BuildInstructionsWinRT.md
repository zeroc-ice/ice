# Building Ice for C++ for WinRT Applications

This file describes the Ice source distribution, including information about
compiler requirements, third-party dependencies, and instructions for building
and testing the distribution. If you prefer, you can download a [Windows
installer][1] that contains pre-compiled debug and release libraries,
executables, and everything else necessary to build Ice applications for WinRT.

## Build Requirements for WinRT

### Operating Systems and Compilers

Ice was extensively tested using the operating systems and compiler versions
listed for our [supported platforms][2].

### Third-Party Libraries

Building Ice for WinRT requires the mcpp pakage:

 - [mcpp][6] 2.7.2 (with patches)

You do not need to build this package yourself, as ZeroC supplies
[Nuget][7] packages for all these third party dependencies.

The Ice build system for Windows downloads and installs Nuget and these 
Nuget packages when you build Ice for C++. The third-party packages
are installed in the ``ice/cpp/third-party-packages`` folder.

## Building Ice

Open a command prompt that is configured for your target architecture. For
example, Visual Studio gives you several alternatives:

- Visual Studio Command Prompt
- Visual Studio x64 Win64 Command Prompt
- Visual Studio x64 Cross Tools Command Prompt

Using the first configuration produces 32-bit binaries, while the second and
third produce 64-bit binaries.

In the command window, change to the `cpp` subdirectory:

    > cd cpp

Edit `config\Make.rules.mak` to establish your build configuration. The comments
in the file provide more information. In particular, you must set `WINRT` to yes
in `Make.rules.mak` or in your environment:

    > set WINRT=yes

Now you're ready to build Ice:

    > nmake /f Makefile.mak

## Installing a C++ Source Build

Simply run `nmake /f Makefile.mak install`. This will install the Ice SDK in the
directory specified by the `prefix` variable in `config\Make.rules.mak`.

## Running the Test Suite

The test suite project requires the [Ice Builder for Visual Studio][8].
Add this extension to Visual Studio before opening the solution.

The WinRT test suite is composed of a set of dynamic libraries (one for each
client/server test) and a GUI application that loads and runs the tests in the
dynamic libraries.

You need to build the dynamic libraries first. Change the working directory:

    > cd cpp\test

Run nmake to build the test libraries:

    > nmake /f Makefile.mak

In Visual Studio, open this solution file:

    cpp\test\TestSuite WinRT.sln

Now select the configuration that matches the settings in
`config\Make.rules.mak` that you used to build the dynamic libraries. For
example, if you built the test libraries for x86 and debug, you must select
Win32 Debug.

After selecting the appropriate configuration, build the solution by choosing
"Build Solution" in the "Build" menu.

After the build completes, you can deploy the application using "Deploy
Solution" in the "Build" menu. Once deployed, you can start the application from
the WinRT Desktop by clicking the "Ice Test Suite" icon.

In the test suite application, selecting "winrt" for the Server field allows you
to run tests with TCP and WS protocols supported by the WinRT server side.

You can also use C++, C# or Java servers to run the tests, which allows you to
use  additional SSL and WSS protocols.

To use servers from C++, C# or Java language mappings, you need to build the
tests for the desired language mapping.

The test controller server is implemented in Java. Refer to the build
instructions in java subdirectory for information on building the test
controller. Use the following command to start the test controller:

    > python scripts/TestController.py

In the "Ice Test Suite" Windows Store application, select the Server language
mapping and Protocol you want to use.

[1]: https://doc.zeroc.com/display/Ice36/Using+the+Windows+Binary+Distribution
[2]: https://doc.zeroc.com/display/Ice36/Supported+Platforms+for+Ice+and+Ice+Touch+3.6.4
[3]: https://libexpat.github.io
[4]: https://github.com/zeroc-ice/bzip2
[5]: http://www.oracle.com/us/products/database/berkeley-db/overview/index.htm
[6]: https://github.com/zeroc-ice/mcpp
[7]: https://zeroc.com/downloads/ice
[8]: https://github.com/zeroc-ice/ice-builder-visualstudio
