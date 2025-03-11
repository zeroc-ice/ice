# Building the Slice Tools for Ice and C++

This page describes how to build the Slice Tools for Ice and C++ from source and package the resulting binaries.

- [Prerequisites](#prerequisites)
- [Building](#building)
- [Building Slice Tools for Ice C++](#building-slice-tools-for-ice-c)

## Prerequisites

1. **.NET SDK 8.0**
   Download the .NET SDK from [dotnet.microsoft.com](https://dotnet.microsoft.com/en-us/download/dotnet).

## Building

Open a command prompt and run:

```sh
dotnet build /p:Platform="Any CPU" /p:Configuration=Release
```

### NuGet Packages

This task is packaged with the Ice for C++ `ZeroC.Ice.Cpp` NuGet package. Refer to the Ice for C++
[build instructions][cpp-building] for more information.

[cpp-building]: ../../BUILDING.md
