# Slice Tools for Ice for C++

ZeroC.Ice.Slice.Tools.Cpp allows you to compile Slice definitions (in `.ice` files) into C++ code (in `.cpp` files)
within MSBuild projects.

This package includes the Slice compiler for C++, `slice2cpp`. This compiler is a native tool with binaries for
Windows (x64).

Once you've added the ZeroC.Ice.Slice.Tools.Cpp NuGet package to your project, the Slice files of your project are
automatically compiled into C++ files every time you build this project.

> The Slice MSBuild task checks the dependencies of the Slice files it compiles and only rebuilds the Slice files that,
> are not up to date.

[Source code][source] | [Package][package] | [slice2cpp options][slice2cpp] | [Slice documentation][slice]

[package]: https://www.nuget.org/packages/ZeroC.Ice.Slice.Tools.Cpp
[slice]: https://doc.zeroc.com/ice/3.7/the-slice-language
[slice2cpp]: https://doc.zeroc.com/ice/latest/language-mappings/c++-mapping/slice2cpp-command-line-options
[source]: https://github.com/zeroc-ice/ice/tree/main/tools/ZeroC.Ice.Slice.Tools.Cpp
