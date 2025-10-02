# Building RPM Packages

This document describes how to build RPM packages for Ice.

## Prerequisites

### System Requirements

- RHEL 9, RHEL 10, or Amazon Linux 2023

### Install Required Repositories

For RHEL 9, and RHEL 10 we first need to ensure all the requires repositories are enabled.
For Amazon Linux 2023 there is no need to install any additional repositories.

#### RHEL 9

```shell
dnf install -y https://zeroc.com/download/ice/3.8/el9/ice-repo-3.8-1.0.0-1.el9.noarch.rpm 
dnf config-manager --set-enabled codeready-builder-for-rhel-9-$(uname -m)-rpms 
```

#### RHEL 10

```shell
dnf install -y https://zeroc.com/download/ice/3.8/el10/ice-repo-3.8-1.0.0-1.el10.noarch.rpm 
dnf config-manager --set-enabled codeready-builder-for-rhel-10-$(uname -m)-rpms 
```

### Install Build Tools

Install the required packaging tools:

```bash
dnf install -y \
    rpmdevtools \
    make \
    curl-minimal \
    git \
    gcc-c++ \
    glibc-devel \
    libstdc++-devel \
    redhat-rpm-config \
    glibc-static \
    libstdc++-static \
    rpm-sign
```

## Building Packages

### 1. Create Build Directory

Create a build directory and clone the Ice repository:

```bash
mkdir -p $HOME/packaging/zeroc-ice
cd $HOME/packaging/zeroc-ice
git clone git@github.com:zeroc-ice/ice.git
mkdir build
```

### 2. Copy Packaging Files

Copy the Debian packaging configuration:

```bash
cp -r ice/packaging/deb/debian build/
```

### 3. Set Version Variables

Extract version information from the changelog:

```bash
export ICE_VERSION=$(dpkg-parsechangelog --file build/debian/changelog --show-field Version)
export UPSTREAM_VERSION=$(echo $ICE_VERSION | cut -f1 -d'-')
```

### 4. Create Source Archive

Generate the upstream tarball from the Git repository:

```bash
cd ice
git archive --format=tar.gz --prefix=zeroc-ice-${UPSTREAM_VERSION}/ \
    -o $HOME/packaging/zeroc-ice/zeroc-ice_${UPSTREAM_VERSION}.orig.tar.gz HEAD
```

### 5. Extract Source

Unpack the source archive in the build directory:

```bash
cd cd $HOME/packaging/zeroc-ice/build
tar xzf ../zeroc-ice_${UPSTREAM_VERSION}.orig.tar.gz --strip-components=1
```

### 6. Install Build Dependencies

Install the required build dependencies:

```bash
sudo mk-build-deps -ir -t 'apt-get -y' debian/control
```

**For Debian 12:** Use the `no-python312` build profile to exclude Python 3.12 support:

```bash
sudo DEB_BUILD_PROFILES="no-python312" mk-build-deps -ir -t 'apt-get -y' debian/control
```

### 7. Build Packages

#### Source Package

Build the source package (generates .dsc and .tar.gz files):

```bash
dpkg-buildpackage -S -uc -us
```

**For Debian 12:** Use the `no-python312` build profile:

```bash
DEB_BUILD_PROFILES="no-python312" dpkg-buildpackage -S -uc -us
```

#### Binary Packages

Build the binary packages:

```bash
dpkg-buildpackage -b -uc -us
```

**For Debian 12:** Use the `no-python312` build profile:

```bash
DEB_BUILD_PROFILES="no-python312" dpkg-buildpackage -b -uc -us
```

The built packages will be available in the parent directory (`$HOME/packaging/zeroc-ice/`):

- Source packages: `*.dsc`, `*.tar.gz`
- Binary packages: `*.deb`

> The `-uc -us` flags skip signing the packages (useful for local builds). Packages can be signed after building using `debsign` if needed for distribution
