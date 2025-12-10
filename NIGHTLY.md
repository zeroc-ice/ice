# Installing the Nightly Builds <!-- omit in toc -->

This document provides instructions for installing Ice from nightly builds.

Ice nightly builds are produced daily from the main GitHub branch.

- [Dev kits](#dev-kits)
  - [Ice for C++](#ice-for-c)
  - [Ice for C#](#ice-for-c-1)
  - [Ice for Java](#ice-for-java)
  - [Ice for JavaScript](#ice-for-javascript)
  - [Ice for MATLAB](#ice-for-matlab)
  - [Ice for PHP](#ice-for-php)
  - [Ice for Python](#ice-for-python)
  - [Ice for Ruby](#ice-for-ruby)
  - [Ice for Swift](#ice-for-swift)
- [Ice Services](#ice-services)
  - [Linux](#linux)
  - [macOS](#macos)
  - [Windows](#windows)
- [Linux Repositories](#linux-repositories)
  - [RPM-based Linux (RHEL, Amazon Linux)](#rpm-based-linux-rhel-amazon-linux)
  - [DEB-based Linux (Debian, Ubuntu)](#deb-based-linux-debian-ubuntu)
- [macOS Brew Formula and Cask](#macos-brew-formula-and-cask)

## Dev kits

### Ice for C++

#### Linux <!-- omit in toc -->

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

#### macOS <!-- omit in toc -->

Ice for C++ is included in the [ice formula](#macos-brew-formula-and-cask).

#### Windows <!-- omit in toc -->

Ice for C++ is distributed as the `ZeroC.Ice.Cpp` NuGet package on Windows.

Add the following feed to your NuGet sources to get the nightly package:

```shell
https://download.zeroc.com/nexus/repository/nuget-nightly/
```

> `ZeroC.Ice.Cpp` includes C++ libraries, header files, the Slice to C++ compiler, and the IceBox service.

### Ice for C#

Ice for C# is distributed as NuGet packages for all platforms.

Add the following feed to your NuGet sources to get the nightly packages:

```shell
https://download.zeroc.com/nexus/repository/nuget-nightly/
```

### Ice for Java

The nightly packages are available from the maven ZeroC maven-nightly repository.

To use them, add the following Maven repository to your build configuration:

```shell
https://download.zeroc.com/nexus/repository/maven-nightly/
```

#### Gradle <!-- omit in toc -->

Here’s an example configuration using Kotlin DSL:

```kotlin
// settings.gradle.kts
pluginManagement {
    repositories {
        gradlePluginPortal()
        // Use the nightly build of the Slice Tools plugin, published to the ZeroC maven-nightly repository.
        maven {
            url = uri("https://download.zeroc.com/nexus/repository/maven-nightly/")
            content {
                includeGroupByRegex("com\\.zeroc.*")
            }
        }
    }
}

dependencyResolutionManagement {
    repositories {
        mavenCentral()
        // Use the nightly build of Ice, published to the ZeroC maven-nightly repository.
        maven {
            url = uri("https://download.zeroc.com/nexus/repository/maven-nightly/")
            content {
                includeGroupByRegex("com\\.zeroc.*")
            }
        }
    }
}
```

### Ice for JavaScript

The nightly NPM package @zeroc/ice is available from the ZeroC NPM nightly registry.

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

Ice toolbox for MATLAB R2025b:

- Windows: [ice-nightly-R2025b-win.mltbx](https://download.zeroc.com/ice/nightly/ice-nightly-R2025b-win.mltbx)
- Ubuntu 24.04: [ice-nightly-R2025b-linux.mltbx](https://download.zeroc.com/ice/nightly/ice-nightly-R2025b-linux.mltbx)

To install a toolbox, download the package corresponding to your platform and simply double-click on the `.mltbx` file
from MATLAB.

### Ice for PHP

#### Linux <!-- omit in toc -->

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

### Ice for Python

#### PyPI <!-- omit in toc -->

The nightly package for all platforms (Linux, macOS, Windows) is available from the ZeroC PyPI nightly repository.

To install the latest zeroc-ice nightly package, add the following lines to your requirements.txt:

```shell
# Use ZeroC nightly repository as the main index for pip
--index-url https://download.zeroc.com/nexus/repository/pypi-nightly/simple/

# Allow installing packages from the official PyPI index if not found in the ZeroC repository
--extra-index-url https://pypi.org/simple/

# Enable installation of pre-release versions (required for nightly builds)
--pre

# Specify the zeroc-ice package (latest nightly version)
zeroc-ice
```

#### Linux <!-- omit in toc -->

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

The nightly Ruby GEM package is available from the ZeroC RubyGems nightly repository.

You can install the latest Ruby gem by running the following command:

```shell
gem install zeroc-ice --source https://download.zeroc.com/nexus/repository/rubygems-nightly --user \
    --pre --version '>= 3.8.0.pre'
```

### Ice for Swift

The nightly package is available via the Swift Package Manager (SPM).

To use it in your project, add the following URL as a dependency in your Package.swift:

```shell
https://github.com/zeroc-ice/ice-swift-nightly
```

## Ice Services

This section describes how to install the DataStorm, Glacier2, IceBox, IceBridge, IceGrid, IceStorm services, and the
associated admin tools.

> DataStorm is primarily a library-based service, installed as part of Ice for C++. The DataStorm component described
> in this section is the DataStorm server (dsnode).

### Linux

First, enable the Ice nightly DNF or APT repository on your system as per [Linux Repositories](#linux-repositories).
Then, install the desired service or tool. For example:

#### RHEL, Amazon Linux <!-- omit in toc -->

```shell
sudo dnf install icegrid
```

Refer to the [RPM packages README] for a complete list of available service packages and their package names.

#### Debian, Ubuntu <!-- omit in toc -->

```shell
sudo apt-get update
sudo apt-get install zeroc-icegrid
```

Refer to the [DEB packages README] for a complete list of available service packages and their package names.

### macOS

All Ice services and admin tools are included in the [ice formula](#macos-brew-formula-and-cask), except IceGrid GUI,
which is provided by a [cask](#macos-brew-formula-and-cask).

### Windows

The Ice services (Glacier2, IceGrid, IceStorm, etc.) and all admin tools for these services (IceGridGUI, icegridadmin,
etc.) are provided in a Windows installer MSI package:

- [Ice-Services-nightly.msi](https://download.zeroc.com/ice/nightly/Ice-Services-nightly.msi)

## Linux Repositories

### RPM-based Linux (RHEL, Amazon Linux)

The nightly RPM packages for Red Hat Enterprise Linux and Amazon Linux are available from the ZeroC DNF nightly
repository.

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

The nightly DEB packages for Debian and Ubuntu are available from the ZeroC APT nightly repository.

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

## macOS Brew Formula and Cask

You can install the nightly ice formula by running:

```shell
brew install zeroc-ice/nightly/ice
```

The ice formula includes:

- the Ice for C++ runtime and dev kit.
- all Ice services (Glacier2, IceGrid, IceStorm, etc.) and all admin tools except IceGrid GUI.

IceGridGUI is distributed in its own brew cask. This cask is not yet available.

[RPM packages README]: packaging/rpm/README
[DEB packages README]: packaging/deb/debian/README
