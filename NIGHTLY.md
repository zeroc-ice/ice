# Installing the Nightly Builds <!-- omit in toc -->

This document provides instructions for installing Ice from nightly builds of Ice.

Ice nightly builds are produced daily from the main GitHub branch.

- [Dev kits](#dev-kits)
  - [Ice for C++](#ice-for-c)
    - [Linux](#linux)
      - [RHEL, Amazon Linux](#rhel-amazon-linux)
      - [Debian, Ubuntu](#debian-ubuntu)
    - [macOS](#macos)
    - [Windows](#windows)
  - [Ice for C#](#ice-for-c-1)
  - [Ice for Java](#ice-for-java)
    - [Gradle](#gradle)
  - [Ice for JavaScript](#ice-for-javascript)
  - [Ice for MATLAB](#ice-for-matlab)
  - [Ice for PHP](#ice-for-php)
    - [Linux](#linux-1)
      - [RHEL](#rhel)
      - [Amazon Linux](#amazon-linux)
      - [Debian, Ubuntu](#debian-ubuntu-1)
    - [macOS](#macos-1)
  - [Ice for Python](#ice-for-python)
    - [Pypi](#pypi)
    - [Linux](#linux-2)
      - [RHEL, Amazon Linux](#rhel-amazon-linux-1)
      - [Debian, Ubuntu](#debian-ubuntu-2)
  - [Ice for Ruby](#ice-for-ruby)
  - [Ice for Swift](#ice-for-swift)
- [Ice Services](#ice-services)
  - [Linux](#linux-3)
    - [RHEL, Amazon Linux](#rhel-amazon-linux-2)
    - [Debian, Ubuntu](#debian-ubuntu-3)
  - [macOS](#macos-2)
  - [Windows](#windows-1)
- [Linux Repositories](#linux-repositories)
  - [RPM-based Linux (RHEL, Amazon Linux)](#rpm-based-linux-rhel-amazon-linux)
  - [DEB-based Linux (Debian, Ubuntu)](#deb-based-linux-debian-ubuntu)
- [macOS Brew Formula](#macos-brew-formula)

## Dev kits

### Ice for C++

#### Linux

First, enable the Ice nightly DNF or APT repository on your system as per [Linux Repositories](#linux-repositories).
Then, install the C++ dev kit:

##### RHEL, Amazon Linux

```shell
sudo dnf install libice-c++-devel
```

##### Debian, Ubuntu

```shell
sudo apt-get update
sudo apt-get install libzeroc-ice-dev
```

#### macOS

Ice for C++ is included in the [ice formula](#macos-brew-formula).

#### Windows

Ice for C++ is distributed as the `ZeroC.Ice.Cpp` NuGet package on Windows.

Add the following feed to your NuGet sources to get the nightly package:

```shell
https://download.zeroc.com/nexus/repository/nuget-nightly/
```

### Ice for C\#

Ice for C# is distributed as NuGet packages for all platforms.

Add the following feed to your NuGet sources to get the nightly packages:

```shell
https://download.zeroc.com/nexus/repository/nuget-nightly/
```

### Ice for Java

The ZeroC Ice Java nightly packages are available from the maven central snapshots repository.

To use them, add the following Maven repository to your build configuration:

```shell
https://central.sonatype.com/repository/maven-snapshots/
```

#### Gradle

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

### Ice for JavaScript

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

### Ice for MATLAB

Ice toolbox for MATLAB R2025a:

- Windows: [ice-nightly-R2025a-win.mltbx](https://download.zeroc.com/ice/nightly/ice-nightly-R2025a-win.mltbx)
- Ubuntu 24.04: [ice-nightly-R2025a-linux.mltbx](https://download.zeroc.com/ice/nightly/ice-nightly-R2025a-linux.mltbx)

To install a toolbox, download the package corresponding to your platform and simply double-click the `.mltbx` file
from MATLAB.

### Ice for PHP

#### Linux

First, enable the Ice nightly DNF or APT repository on your system as per [Linux Repositories](#linux-repositories).
Then, install the Ice for PHP package:

##### RHEL

```shell
sudo dnf install php-ice
```

##### Amazon Linux

```shell
sudo dnf install php8.4-ice
```

##### Debian, Ubuntu

```shell
sudo apt-get update
sudo apt-get install php-zeroc-ice
```

#### macOS

Not yet available.

### Ice for Python

#### PyPI

The ZeroC Ice nightly Python package is available from the ZeroC PyPI nightly repository for all platforms (Linux,
macOS, Windows).

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

#### Linux

Ice for Python is also available as a RPM or DEB package, depending on your distribution.

First, enable the Ice nightly DNF or APT repository on your system as per [Linux Repositories](#linux-repositories).
Then, install the Ice for Python package:

##### RHEL, Amazon Linux

```shell
sudo dnf install python3-ice
```

##### Debian, Ubuntu

```shell
sudo apt-get update
sudo apt-get install python3-zeroc-ice
```

### Ice for Ruby

The ZeroC Ice nightly Ruby GEM package is available from the ZeroC RubyGems nightly repository.

You can install the latest Ruby gem by running the following command:

```shell
gem install zeroc-ice --source https://download.zeroc.com/nexus/repository/rubygems-nightly --user \
    --pre --version '>= 3.8.0.pre'
```

### Ice for Swift

The Ice for Swift nightly package is available via Swift Package Manager (SPM) from the zeroc-ice/ice-swift-nightly
GitHub repository.

To use it in your project, add the following URL as a dependency in your Package.swift:

```shell
https://github.com/zeroc-ice/ice-swift-nightly
```

## Ice Services

This section describes how to install the DataStorm, Glacier2, IceBox, IceBridge, IceGrid, and IceStorm services.

> DataStorm is primarily a library-based service, installed as part of Ice for C++. The DataStorm component described
> in this section in the DataStorm server (dsnode).

### Linux

First, enable the Ice nightly DNF or APT repository on your system as per [Linux Repositories](#linux-repositories).
Then, install the desired service. For example:

#### RHEL, Amazon Linux

```shell
sudo dnf install icegrid
```

Refer to the [RPM packages README] for the package names.

#### Debian, Ubuntu

```shell
sudo apt-get update
sudo apt-get install zeroc-icegrid
```

Refer to the [DEB packages README] for the package names.

### macOS

All Ice services are included in the [ice formula](#macos-brew-formula).

### Windows

The Ice services (Glacier2, IceGrid, IceStorm, etc.) and all admin tools for these services (IceGridGUI, icegridadmin,
etc.) are provided in a Windows installer MSI package:

- [Ice-nightly.msi](https://download.zeroc.com/ice/nightly/Ice-nightly.msi)

## Linux Repositories

### RPM-based Linux (RHEL, Amazon Linux)

The ZeroC Ice nightly RPM packages for Red Hat Enterprise Linux and Amazon Linux are available from the ZeroC DNF
nightly repository.

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

See also: [RPM packages README]

### DEB-based Linux (Debian, Ubuntu)

The ZeroC Ice nightly DEB packages for Debian and Ubuntu are available from the ZeroC APT nightly repository.

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

See also: [DEB packages README]

## macOS Brew Formula

You can install the nightly ice formula by running:

```shell
brew install zeroc-ice/nightly/ice
```

The ice formula includes:

- the Ice for C++ runtime and dev kit
- all Ice services (Glacier2, IceGrid, IceStorm, etc.)

[RPM packages README]: packaging/rpm/README
[DEB packages README]: packaging/deb/debian/README
