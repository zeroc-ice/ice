# Building Slice Tools for Ice and C++

This page describes how to build Slice Tools for Ice and C++ from source and package the resulting binaries.

- [Prerequisites](#prerequisites)
- [Building](#building)
- [Building Slice Tools for Ice C++](#building-slice-tools-for-ice-c)

## Prerequisites

1. **.NET SDK 8.0**  
   Download the .NET SDK from [dotnet.microsoft.com](https://dotnet.microsoft.com/en-us/download/dotnet).

## Building Slice Tools for Ice and C++

Open a command prompt and run:

```sh
dotnet build /p:Platform="Any CPU" /p:Configuration=Release
```

### NuGet Packages

This task is packaged with the Ice for C++ `ZeroC.Ice.Slice.Tools.Cpp` NuGet package. Refer to building instructions for
the [Ice for C++](../../BUILDING.md) for more information.
