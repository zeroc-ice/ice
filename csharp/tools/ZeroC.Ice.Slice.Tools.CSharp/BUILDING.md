# Building Slice Tools for Ice and C#

This page describes how to build Slice Tools for Ice .NET from source and package the resulting binaries.

- [Prerequisites](#prerequisites)
- [Building](#building)
- [NuGet Package](#nuget-package)

## Prerequisites

1. **.NET SDK 8.0**  
   Download the .NET SDK from [dotnet.microsoft.com](https://dotnet.microsoft.com/en-us/download/dotnet).

2. **The Slice to C# compiler from the C++ source distribution**  
   Refer to the [build instructions](../../cpp/BUILDING.md) in the `cpp` folder for details on how to build the C++
   source distribution.

### Building Slice Tools for Ice .NET

Open a command prompt and run:

```sh
dotnet build /p:Platform="Any CPU" /p:Configuration=Release
```

### NuGet Package

To create the NuGet package, open a command prompt and run the following command:

```sh
dotnet pack /p:Platform="Any CPU" /p:Configuration=Release
```

This command creates the `ZeroC.Ice.Slice.Tools.CSharp` NuGet package in the project output directory.

On Linux and macOS, the NuGet package is located at:

```shell
bin/Release/ZeroC.Ice.Slice.Tools.CSharp.3.8.0-alpha0.nupkg
```

On Windows, the NuGet package is located at:

```shell
bin\Release\ZeroC.Ice.Slice.Tools.CSharp.3.8.0-alpha0.nupkg
```

By default, the NuGet package includes the Slice-to-CSharp compiler (`slice2cs`) for the current platform. To include
the Slice-to-CSharp for all supported platforms, you must copy the required binaries to the staging directory before
running the `dotnet pack` command.

The build expects the staging directory to be at the location specified by the `SLICE2CS_STAGING_PATH` environment
variable. The staging directory should have the following structure:

```shell
$(SLICE2CS_STAGING_PATH)/linux-arm64/slice2cs
$(SLICE2CS_STAGING_PATH)/linux-x64/slice2cs
$(SLICE2CS_STAGING_PATH)/macos-arm64/slice2cs
$(SLICE2CS_STAGING_PATH)/windows-x64/slice2cs.exe
```
