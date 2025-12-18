# iceboxnet

The IceBox server for .NET.

The IceBox server loads and manages [IceBox services], allowing you to build Ice servers as collections of services
that can be dynamically loaded through configuration.

[Package][package] | [Source code][source] | [Examples][examples] | [Documentation][docs]

## Installation

To install **iceboxnet** as a .NET tool in the current project, run:

```shell
dotnet tool install iceboxnet --create-manifest-if-needed
```

See the [dotnet tool install] documentation for additional install options.

## Usage

Once installed, you can start the IceBox server with:

```shell
dotnet iceboxnet --Ice.Config=icebox.config
```

[docs]:https://docs.zeroc.com/ice/3.8/csharp/
[examples]: https://github.com/zeroc-ice/ice-demos/tree/3.8/csharp
[package]: https://www.nuget.org/packages/iceboxnet
[IceBox services]: https://www.nuget.org/packages/ZeroC.IceBox
[source]: https://github.com/zeroc-ice/ice/tree/3.8/csharp/src/iceboxnet
[dotnet tool install]: https://learn.microsoft.com/en-gb/dotnet/core/tools/dotnet-tool-install
