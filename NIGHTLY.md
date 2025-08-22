# Installing the Nightly Builds

This document provides instruction for installing Ice nightly builds. Ice nightly builds are produced daily from
GitHub main branch and target the next Ice non patch release.

- [Linux](#linux)
  - [RPM-based Linux (RHEL, Amazon Linux)](rpm-based-linux-rhel-amazon-linux)
  - [DEB-based Linux (Debian, Ubuntu)](deb-based-linux-debian-ubuntu)
- [macOS](#macos)
  - [Swift](#swift)
  - [Homebrew](#homebrew)
- [Windows](#windows)
  - [C++ NuGet](#c-nuget)
  - [MSI Installer](#msi-installer)
- [C#](c)
- [JavaScript](#javascript)
- [Java](#java)
- [MATLAB](#matlab)
- [Python](#python)
- [Ruby](#ruby)

## Linux

### RPM-based Linux (RHEL, Amazon Linux)

The ZeroC Ice nightly RPM packages for Red Hat Enterprise Linux 9, Red Hat Enterprise Linux 10, and Amazon Linux 2023
are available from the ZeroC DNF nightly repository.

To enable the nightly repository for your distribution run:

- Red Hat Enterprise Linux 9

  ```shell
  sudo dnf install https://download.zeroc.com/ice/nightly/el9/ice-repo-nightly-1.0.0-1.el9.noarch.rpm
  ```

- Red Hat Enterprise Linux 10

  ```shell
  sudo dnf install https://download.zeroc.com/ice/nightly/el10/ice-repo-nightly-1.0.0-1.el10.noarch.rpm
  ```

- Amazon Linux 2023

  ```shell
  sudo dnf install https://download.zeroc.com/ice/nightly/amzn2023/ice-repo-nightly-1.0.0-1.amzn2023.noarch.rpm
  ```

Then install the desired packages. For example:

```shell
sudo dnf install libice-c++-devel
```

For a full list of available packages, see the [RPM packages README](packaging/rpm/README).

### DEB-based Linux (Debian, Ubuntu)

The ZeroC Ice nightly DEB packages for Debian 12, Debian 13, and Ubuntu 24.04 are available from the ZeroC APT nightly
repository.

To enable the nightly repository for your distribution run:

- Debian 12

  ```shell
  wget "https://download.zeroc.com/ice/nightly/debian12/ice-repo-nightly_1.0_all.deb" -O ice-repo-nightly.deb
  sudo dpkg -i ice-repo-nightly.deb
  rm ice-repo-nightly.deb
  ```

- Debian 13

  ```shell
  wget "https://download.zeroc.com/ice/nightly/debian13/ice-repo-nightly_1.0_all.deb" -O ice-repo-nightly.deb
  sudo dpkg -i ice-repo-nightly.deb
  rm ice-repo-nightly.deb
  ```

- Ubuntu 24.04

  ```shell
  wget "https://download.zeroc.com/ice/nightly/ubuntu24.04/ice-repo-nightly_1.0_all.deb" -O ice-repo-nightly.deb
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

- [Ice-nightly.msi](https://download.zeroc.com/ice/nightly/Ice-nightly.msi)

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

The ZeroC Ice Java nightly packages are available from the maven central snapshots repository.

To use them, add the following Maven repository to your build configuration:

```shell
https://central.sonatype.com/repository/maven-snapshots/
```

### Gradle

Here’s an example configuration using Kotlin DSL:

```kotlin
// settings.gradle.kts
pluginManagement {
    repositories {
        mavenLocal()
        maven("https://central.sonatype.com/repository/maven-snapshots/")
        gradlePluginPortal() // Keep this to allow fetching other plugins
    }
}

dependencyResolutionManagement {
    repositories {
        // This demo uses the latest Ice nightly build published to the maven central snapshots repository.
        maven("https://central.sonatype.com/repository/maven-snapshots/")
        mavenCentral()
    }
}
```

## MATLAB

The ZeroC Ice MATLAB nightly toolboxes for Windows and Ubuntu 24.04 are available for direct download from the ZeroC
NuGet nightly repository.

You can download and install the appropriate .mltbx file for your platform:

- Windows: [ice-nightly-R2024a-win.mltbx](https://download.zeroc.com/ice/nightly/ice-nightly-R2025a-win.mltbx)
- Ubuntu 24.04: [ice-nightly-R2024a-linux.mltbx](https://download.zeroc.com/ice/nightly/ice-nightly-R2025a-linux.mltbx)

To install, simply double-click the `.mltbx` file in MATLAB.

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
