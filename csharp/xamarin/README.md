# Building the Ice for Xamarin Test Suite

This directory contains Visual Studio projects for building the Ice test
suite as a Xamarin application that can be deployed to iOS, Android or UWP
platforms.

The Xamarin test suite use the Ice assemblies for .NET Standard 2.0. either
from the source distribution that must be build before this application or
using the zeroc.ice.net NuGet package.

## Building on Windows

### Windows Build Requirements

* Visual Studio 2017 with following workloads:
  * Universal Windows Platform development
  * Mobile development with .NET
  * .NET Core cross-platform development

### Building test applications

Open and build `controller.sln` solution file wiht Visual Studio 2017

### Running the Android test suite

```
set PATH=%LOCALAPPDATA%\Android\sdk\tools;%PATH%
set PATH=%LOCALAPPDATA%\Android\sdk\tools\bin;%PATH%
set PATH=%LOCALAPPDATA%\Android\sdk\platform-tools;%PATH%
set PATH=%LOCALAPPDATA%\Android\sdk\emulator;%PATH%

cd csharp\xamarin\android
python allTest.py --androidemulator --controller-app --config Release --platform x64
```

### Running the UWP test suite

```
cd csharp\xamarin\uwp
python allTest.py --controller-app --config Release --platform x64
```

## Building on macOS

### macOS Build Requirements

* Visual Studio for Mac

### Building test applications

Open and build `controller.sln` solution file wiht Visual Studio for Mac

### Running the Android test suite

```
export PATH=~/Library/Android/sdk/tools/bin:$PATH
export PATH=~/Library/Android/sdk/platform-tools:$PATH
export PATH=~/Library/Android/sdk/emulator:$PATH

cd csharp\xamarin\android
python allTest.py --androidemulator --controller-app --config Release --platform x64
```

### Running the iOS test suite

```
cd csharp\xamarin\ios
python allTest.py --controller-app --platform iphonesimulator --config Release
```
