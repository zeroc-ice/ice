# Building the Windows Installer

This document explains how to build the **Ice Services** installer for Windows.

First build the C++ distribution. From the `cpp` directory, follow `cpp/BUILDING.md` to build:

- **Platform:** x64
- **Configuration:** Release

Next build IceGridGUI. From the `java` directory, follow `java/BUILDING.md` to build the Java components, including
**IceGridGUI**.

## Download the VC++ Redistributable

Download `vc_redist.x64.exe` from Microsoft and place it in `packaging\windows-installer\redist\`:

```cmd
mkdir packaging\windows-installer\redist
curl -L -o packaging\windows-installer\redist\vc_redist.x64.exe https://aka.ms/vs/17/release/vc_redist.x64.exe
```

## Build the MSI

```cmd
cd packaging\windows-installer
dotnet build -c Release -p:Platform=x64
```

## Build the Bundle Installer

```cmd
cd packaging\windows-installer\bundle
dotnet build -c Release -p:Platform=x64
```

The installer will be placed in:

```cmd
packaging\windows-installer\bundle\bin\x64\Release\
```
