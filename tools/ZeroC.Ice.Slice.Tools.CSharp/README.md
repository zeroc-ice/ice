# Slice Tools for Ice C\#

ZeroC.Ice.Slice.Tools.CSharp allows you to compile Slice definitions (in `.ice` files) into C# code (in `.cs` files)
within MSBuild projects.

This package includes the Slice compiler for C#, `slice2cs`. This compiler is a native tool with binaries for
Linux (x64 and arm64), macOS (x64 and arm64) and Windows (x64).

Once you've added the ZeroC.Ice.Slice.Tools.CSharp NuGet package to your project, the Slice files of your project are
automatically compiled into C# files every time you build this project.

> The Slice MSBuild task checks the dependencies of the Slice files it compiles and only rebuilds the Slice files that,
> are not up to date.

[Source code][source] | [Package][package] | [slice2cs options][slice2cs] | [Slice documentation][slice]

[package]: https://www.nuget.org/packages/ZeroC.Ice.Slice.Tools.CSharp
[slice]: https://doc.zeroc.com/ice/3.7/the-slice-language
[slice2cs]: https://doc.zeroc.com/ice/3.7/language-mappings/c-sharp-mapping/client-side-slice-to-c-sharp-mapping/slice2cs-command-line-options
[source]: https://github.com/zeroc-ice/ice/tree/main/tools/ZeroC.Ice.Slice.Tools.CSharp
