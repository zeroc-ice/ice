# Installing the Nightly Builds

This document provides instruction for installing Ice nightly builds. Ice nightly builds are produced daily from
GitHub main branch and target the next Ice non patch release.

- Linux
  - [RPM-based Linux (RHEL 9/10, Amazon Linux 2023)](rpm-based-linux-rhel-910-amazon-linux-2023)
  - [DEB-based Linux (Debian 12, Ubuntu 24.04)](deb-based-linux-debian-12-ubuntu-2404)
- macOS
  - [Swift](#swift)
  - [Homebrew](#homebrew)
- Windows
  - [C++ NuGet](#c-nuget)
  - [MSI Installer](#msi-installer)
- [C#]()
- [JavaScript](#javascript)
- [Java](#java)
- [MATLAB](#matlab)
- [Python](#python)
- [Ruby](#ruby)

## Linux

### RPM-based Linux (RHEL 9/10, Amazon Linux 2023)

The ZeroC Ice nightly RPM packages for Red Hat Enterprise Linux 9, Red Hat Enterprise Linux 10, and Amazon Linux 2023
are available from the ZeroC DNF nightly repository.

To enable the nightly repository, set your distribution identifier and run:

```shell
DIST=el9  # or 'el10' or 'amzn2023'

sudo dnf install https://download.zeroc.com/ice/nightly/$DIST/ice-repo-nightly-1.0.0-1.$DIST.noarch.rpm
```

Then install the desired packages. For example:

```shell
sudo dnf install libice-c++-devel
```

For a full list of available packages, see the [RPM packages README](packaging/rpm/README).

### DEB-based Linux (Debian 12, Ubuntu 24.04)

The ZeroC Ice nightly DEB packages for Debian 12 and Ubuntu 24.04 are available from the ZeroC APT nightly repository.

To enable the nightly repository, set your distribution name and run:

```shell
# Supported values: debian12 or ubuntu24.04
DIST=debian12  # or 'ubuntu24.04'

wget "https://download.zeroc.com/ice/nightly/$DIST/ice-repo-nightly_1.0_all.deb" -O ice-repo-nightly.deb
sudo dpkg -i ice-repo-nightly.deb
rm ice-repo-nightly.deb
```

Then update your package index and install the desired packages. For example:

```shell
sudo apt-get update
sudo apt-get install libzeroc-ice-dev
```

For a full list of available packages, see the [DEB packages README](packaging/deb/debian/README).

## macOS

### Homebrew

The Ice nightly Homebrew tap is provided by the zeroc-ice/homebrew-nightly repository.

You can install the nightly Ice package by running:

```shell
brew install zeroc-ice/nightly/ice
```

### Swift

The Ice for Swift nightly package is available via Swift Package Manager (SPM) from the zeroc-ice/ice-swift-nightly GitHub repository.

To use it in your project, add the following URL as a dependency in your Package.swift:

```shell
https://github.com/zeroc-ice/ice-swift-nightly
```

## Windows

### C++ NuGet

The ZeroC Ice nightly C++ NuGet packages for Windows are available from the ZeroC NuGet nightly feed.

To use them, add the following feed to your NuGet sources:

```shell
https://download.zeroc.com/nexus/repository/nuget-nightly/
```

Then add the `ZeroC.Ice.Cpp` package to your Visual Studio C++ projects.

### MSI Installer

The ZeroC Ice nightly Windows installer (MSI) is available at:

- [Ice-nightly.msi](https://download.zeroc.com/nexus/repository/nuget-nightly/Ice-nightly.msi)

## C\#

The ZeroC Ice nightly .NET NuGet packages are available from the ZeroC NuGet nightly feed.

To use them, add the following feed to your NuGet sources:

```shell
https://download.zeroc.com/nexus/repository/nuget-nightly/
```

## JavaScript

The ZeroC Ice NPM nightly package @zeroc/ice is available from the ZeroC NPM nightly registry.

To install it, add the following line to your project’s .npmrc file:

```shell
# Use ZeroC nightly registry for @zeroc packages
@zeroc:registry=https://download.zeroc.com/nexus/repository/npm-nightly/
```

Then add the `@zeroc/ice` package to your project:

```shell
npm install @zeroc/ice
```

## Java

The ZeroC Ice Java nightly packages are available from the ZeroC Maven nightly repository.

To use them, add the following Maven repository to your build configuration:

```shell
https://download.zeroc.com/nexus/repository/maven-nightly/
```

## MATLAB

The ZeroC Ice MATLAB nightly toolboxes for Windows and Ubuntu 24.04 are available for direct download from the ZeroC NuGet nightly repository.

You can download and install the appropriate .mltbx file for your platform:

- Windows: [ice-nightly-R2024a-win.mltbx](https://download.zeroc.com/nexus/repository/nuget-nightly/ice-nightly-R2024a-win.mltbx)
- Ubuntu 24.04: [ice-nightly-R2024a-linux.mltbx](https://download.zeroc.com/nexus/repository/nuget-nightly/ice-nightly-R2024a-linux.mltbx)

To install, simply double-click the `.mltbx` file in MATLAB.

### Gradle

Here’s an example configuration using Kotlin DSL:

```kotlin
// settings.gradle.kts
pluginManagement {
    repositories {
        mavenLocal()
        maven("https://download.zeroc.com/nexus/repository/maven-nightly/")
        gradlePluginPortal() // Keep this to allow fetching other plugins
    }
}

dependencyResolutionManagement {
    repositories {
        // This demo uses the latest Ice nightly build published in ZeroC's maven-nightly repository.
        maven("https://download.zeroc.com/nexus/repository/maven-nightly/")
        mavenCentral()
    }
}
```

## Python

The ZeroC Ice nightly Python package is available from the ZeroC PyPI nightly repository.

To install the latest zeroc-ice nightly package, add the following lines to your requirements.txt:

```shell
# Use ZeroC Nightly repository as the main index for pip
--index-url https://download.zeroc.com/nexus/repository/pypi-nightly/simple/

# Allow installing packages from the official PyPI index if not found in the ZeroC repository
--extra-index-url https://pypi.org/simple/

# Enable installation of pre-release versions (required for nightly builds)
--pre

# Specify the zeroc-ice package (latest nightly version)
zeroc-ice
```

## Ruby

The ZeroC Ice nightly Ruby GEM package is available from the ZeroC RubyGems nightly repository.

You can install the latest Ruby gem by running the following command:

```shell
gem install zeroc-ice --source https://download.zeroc.com/nexus/repository/rubygems-nightly --user \
    --pre --version '>= 3.8.0.pre'
```
