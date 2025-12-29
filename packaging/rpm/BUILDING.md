# Building RPM Packages

This document describes how to build RPM packages for Ice.

## Prerequisites

### System Requirements

- RHEL 9, RHEL 10, or Amazon Linux 2023

### Enable Required Repositories

For RHEL 9 and RHEL 10, ensure the required repositories are enabled.
For Amazon Linux 2023, no additional repos are needed.

#### RHEL 9

```bash
sudo dnf install -y https://zeroc.com/download/ice/nightly/3.8/el9/ice-repo-3.8-nightly-1.0.0-1.el9.noarch.rpm
sudo dnf config-manager --set-enabled codeready-builder-for-rhel-9-$(arch)-rpms
```

#### RHEL 10

```bash
sudo dnf install -y https://zeroc.com/download/ice/nightly/3.8/el10/ice-repo-3.8-nightly-1.0.0-1.el10.noarch.rpm
sudo dnf config-manager --set-enabled codeready-builder-for-rhel-10-$(arch)-rpms
```

### Install Build Tools

Install the basic packaging tools:

```bash
sudo dnf install -y \
    rpmdevtools \
    make \
    curl-minimal \
    git \
    rpm-build \
    rpm-sign
```

## Building Packages

### 1. Create Build Directory

Create a build directory and clone the Ice repository:

```bash
mkdir -p $HOME/zeroc-ice
cd $HOME/zeroc-ice
git clone git@github.com:zeroc-ice/ice.git
```

Set up the RPM build tree in the current directory:

```bash
mkdir -p rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
```

### 2. Copy Packaging Files

Copy the package spec file:

```bash
cp ice/packaging/rpm/ice.spec rpmbuild/SPECS/
```

### 3. Download Source Archive

Download the required sources:

```bash
spectool -g --define "_topdir $HOME/zeroc-ice/rpmbuild" \
    --sourcedir $HOME/zeroc-ice/rpmbuild/SPECS/ice.spec
```

### 4. Install Build Dependencies

Install the required build dependencies:

```bash
sudo dnf builddep -y $HOME/zeroc-ice/rpmbuild/SPECS/ice.spec
```

### 5. Build Packages

#### Source Package

Build the source package (generates .src.rpm file):

```bash
rpmbuild -bs --define "_topdir $HOME/zeroc-ice/rpmbuild" \
    $HOME/zeroc-ice/rpmbuild/SPECS/ice.spec
```

#### Binary Packages

Build the binary packages:

```bash
rpmbuild -bb --define "_topdir $HOME/zeroc-ice/rpmbuild" \
    $HOME/zeroc-ice/rpmbuild/SPECS/ice.spec
```

The built packages will be available in:

- Source packages: `$HOME/zeroc-ice/rpmbuild/SRPMS/`
- Binary packages: `$HOME/zeroc-ice/rpmbuild/RPMS/`

> Packages can be signed after building using `rpm --addsign` if needed for distribution
