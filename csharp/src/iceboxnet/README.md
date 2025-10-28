# iceboxnet

IceBox server for .NET.
The IceBox server loads and manages [IceBox services], allowing you to build Ice servers as collections of services
that can be dynamically configured.

[Package][package] | [Source code][source] | [Examples][examples] | [Documentation][docs]

## Installation

Install **iceboxnet** as a .NET tool using the following command:

```shell
dotnet tool install iceboxnet --prerelease --create-manifest-if-needed
```

## Usage

Once installed, you can start the IceBox server with:

```shell
dotnet iceboxnet --Ice.Config=icebox.config
```

[docs]:https://docs.zeroc.com/ice/latest/csharp/
[examples]: https://github.com/zeroc-ice/ice-demos/tree/main/csharp
[package]: https://www.nuget.org/packages/iceboxnet
[IceBox services]: https://www.nuget.org/packages/ZeroC.IceBox
[source]: https://github.com/zeroc-ice/ice/tree/main/csharp/src/iceboxnet
