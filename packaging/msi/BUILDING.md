# Building MSI Packages

This document explains how to build the **Ice Services** MSI package for Windows.

First build the C++ distribution. From the `cpp` directory, follow `cpp/BUILDING.md` to build:

- **Platform:** x64
- **Configuration:** Release

Next build IceGridGUI. From the `java` directory, follow `java/BUILDING.md` to build the Java components, including
**IceGridGUI**.

Finally build the MSI Installer

```cmd
cd packaging\msi
dotnet build -c Release -p:Platform=x64
```

The MSI will be produced at:

```cmd
packaging\msi\bin\x64\Release\
```
