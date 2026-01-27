# Installing the Nightly Builds <!-- omit in toc -->

This document describes how to install the nightly builds for the 3.7 branch, which are expected to become the next
3.7 patch release.

Ice 3.7 nightly builds are produced daily from the 3.7 GitHub branch.

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

Ice for C++ is distributed as NuGet packages on Windows.

- `ZeroC.Ice.v142` compatible with Visual Studio 2019
- `ZeroC.Ice.v143` compatible with Visual Studio 2022 and later

Add the following feed to your NuGet sources to get the nightly packages:

```shell
https://download.zeroc.com/nexus/repository/nuget-3.7-nightly/
```

> `ZeroC.Ice.v142` and `ZeroC.Ice.v143` packages include C++ libraries, header files, the Slice to C++ compiler,
> all the Ice services (Glacier2, IceGrid, etc.), and the command-line admin tools for these services.

### Ice for C#

Ice for C# is distributed as a NuGet package `ZeroC.Ice.Net` for all platforms.

Add the following feed to your NuGet sources to get the nightly package:

```shell
https://download.zeroc.com/nexus/repository/nuget-3.7-nightly/
```

### Ice for Java

The nightly packages are available from the ZeroC `maven-3.7-nightly` Maven repository.

To use them, add the following Maven repository to your build configuration:

```shell
https://download.zeroc.com/nexus/repository/maven-3.7-nightly/
```

#### Gradle <!-- omit in toc -->

Here’s an example configuration using Kotlin DSL:

```kotlin
// settings.gradle.kts
dependencyResolutionManagement {
    repositories {
        mavenCentral()
        // Use the nightly build of Ice, published to the ZeroC maven-3.7-nightly repository.
        maven {
            url = uri("https://download.zeroc.com/nexus/repository/maven-3.7-nightly/")
            content {
                includeGroupByRegex("com\\.zeroc.*")
            }
        }
    }
}
```

### Ice for JavaScript

The nightly NPM packages `ice` and `slice2js` are available from the ZeroC NPM nightly registry.

To use them, add the following line to your project’s .npmrc file:

```shell
# Use ZeroC nightly registry for all packages
registry=https://download.zeroc.com/nexus/repository/npm-3.7-nightly/
```

> This registry also proxies packages from npmjs.org, so your other dependencies will install normally.

Then add the `slice2js` and `ice` packages to your project:

```shell
npm install --save-dev slice2js
npm install ice
```

### Ice for MATLAB

Ice toolbox for MATLAB R2025b:

- Windows: [ice-nightly-R2025b-win.mltbx](https://download.zeroc.com/ice/nightly/3.7/ice-nightly-R2025b-win.mltbx)
- Ubuntu 24.04: [ice-nightly-R2025b-linux.mltbx](https://download.zeroc.com/ice/nightly/3.7/ice-nightly-R2025b-linux.mltbx)

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

To install the latest zeroc-ice 3.7 nightly package:

```shell
pip install --pre --index-url https://download.zeroc.com/nexus/repository/pypi-3.7-nightly/simple/ zeroc-ice
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
gem install zeroc-ice --source https://download.zeroc.com/nexus/repository/rubygems-3.7-nightly --user \
    --pre --version '>= 3.7.11.pre'
```

### Ice for Swift

The nightly package is available via the Swift Package Manager (SPM).

To use it in your project, add the following package as a dependency in your Package.swift:

```shell
.package(url: "https://github.com/zeroc-ice/ice-swift-nightly.git", branch: "3.7")
```

## Ice Services

This section describes how to install the Glacier2, IceBox, IceBridge, IceGrid, IceStorm services, and the
associated admin tools.

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

The Ice services (Glacier2, IceGrid, IceStorm, etc.) and admin tools for these services are available in both the
Windows installer MSI package and the C++ NuGet packages listed in the [Ice for C++](#ice-for-c) Windows section.
IceGridGUI is only included in the MSI package:

- [Ice-nightly.msi](https://download.zeroc.com/ice/nightly/3.7/Ice-nightly.msi)

## Linux Repositories

### RPM-based Linux (RHEL, Amazon Linux)

The nightly RPM packages for Red Hat Enterprise Linux and Amazon Linux are available from the ZeroC DNF nightly
repository.

To enable the nightly repository for your distribution run:

- Red Hat Enterprise Linux 9

  ```shell
  sudo dnf install https://download.zeroc.com/ice/nightly/3.7/el9/ice-repo-3.7-nightly-1.0.0-1.el9.noarch.rpm
  ```

- Red Hat Enterprise Linux 10

  ```shell
  sudo dnf install https://download.zeroc.com/ice/nightly/3.7/el10/ice-repo-3.7-nightly-1.0.0-1.el10.noarch.rpm
  ```

- Amazon Linux 2023

  ```shell
  sudo dnf install https://download.zeroc.com/ice/nightly/3.7/amzn2023/ice-repo-3.7-nightly-1.0.0-1.amzn2023.noarch.rpm
  ```

See also: [RPM packages README]

### DEB-based Linux (Debian, Ubuntu)

The nightly DEB packages for Debian and Ubuntu are available from the ZeroC APT nightly repository.

To enable the nightly repository for your distribution run:

- Debian 12

  ```shell
  wget "https://download.zeroc.com/ice/nightly/3.7/debian12/ice-repo-3.7-nightly_1.0.0_all.deb" -O ice-repo-3.7-nightly.deb
  sudo dpkg -i ice-repo-3.7-nightly.deb
  rm ice-repo-3.7-nightly.deb
  ```

- Debian 13

  ```shell
  wget "https://download.zeroc.com/ice/nightly/3.7/debian13/ice-repo-3.7-nightly_1.0.0_all.deb" -O ice-repo-3.7-nightly.deb
  sudo dpkg -i ice-repo-3.7-nightly.deb
  rm ice-repo-3.7-nightly.deb
  ```

- Ubuntu 24.04

  ```shell
  wget "https://download.zeroc.com/ice/nightly/3.7/ubuntu24.04/ice-repo-3.7-nightly_1.0.0_all.deb" -O ice-repo-3.7-nightly.deb
  sudo dpkg -i ice-repo-3.7-nightly.deb
  rm ice-repo-3.7-nightly.deb
  ```

- Ubuntu 22.04

  ```shell
  wget "https://download.zeroc.com/ice/nightly/3.7/ubuntu22.04/ice-repo-3.7-nightly_1.0.0_all.deb" -O ice-repo-3.7-nightly.deb
  sudo dpkg -i ice-repo-3.7-nightly.deb
  rm ice-repo-3.7-nightly.deb
  ```

See also: [DEB packages README]

## macOS Brew Formula and Cask

You can install the nightly ice@3.7 formula by running:

```shell
brew install zeroc-ice/nightly/ice@3.7
```

The ice@3.7 formula includes:

- the Ice for C++ runtime and dev kit.
- all Ice services (Glacier2, IceGrid, IceStorm, etc.) and all admin tools except IceGrid GUI.

IceGridGUI is distributed in its own brew cask. This cask is not yet available.

[RPM packages README]: packaging/rpm/README
[DEB packages README]: packaging/deb/debian/README
