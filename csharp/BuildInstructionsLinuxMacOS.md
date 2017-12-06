# Building Ice for .NET on Linux and macOS

This page describes how to build Ice for .NET from source using the .NET Core SDK.
As an alternative, you can download and install the [zeroc.ice.net][1] NuGet package.

A source build of Ice for .NET on Linux or macOS produces assemblies for [.NET Standard 2.0][2].

## Build Requirements

Ice for .NET was extensively tested using the operating systems and compiler
versions listed for our [supported platforms][3].

You need the [.NET Core 2.0 SDK][4] to build Ice for .NET from source.

## Compiling Ice for .NET with .NET Core SDK

Open a command prompt and change to the `csharp` subdirectory:
```
cd csharp
```

To build all Ice assemblies and the associated test suite, run:
```
dotnet msbuild msbuild/ice.proj
```

Upon completion, the Ice assemblies for .NET Standard 2.0 are placed in the
`lib/netstandard2.0` directory.

You can skip the build of the test suite with the `BuildDist` target:
```
dotnet msbuild msbuild/ice.proj /t:BuildDist
```

## Running the Tests

*There is a bug in .NET Core 2.0.2 Socket implementation that can cause failures when
running the test suite. This bug is fixed in the .NET Core 2.0.3 release available
from [.NET Core download page][4].*

Python is required to run the test suite. Additionally, the Glacier2 tests
require the Python module `passlib`, which you can install with the command:
```
pip install passlib
```

To run the tests, open a command window and change to the top-level directory.
At the command prompt, execute:
```
python allTests.py
```

If everything worked out, you should see lots of `ok` messages. In case of a
failure, the tests abort with `failed`.

## NuGet packages

*Temporary limitation: you currently cannot create NuGet packages on Linux and macOS.*

[1]: https://zeroc.com/distributions/ice
[2]: https://blogs.msdn.microsoft.com/dotnet/2017/08/14/announcing-net-standard-2-0
[3]: https://doc.zeroc.com/display/Rel/Supported+Platforms+for+Ice+3.7.0
[4]: https://www.microsoft.com/net/download
[5]: https://github.com/dotnet/core-setup#daily-builds
