#!/bin/bash
set -eux  # Exit on error, print commands

RPM_BUILD_ROOT=/workspace/build

mkdir -p "$RPM_BUILD_ROOT"
cd "$RPM_BUILD_ROOT"

mkdir -p BUILD BUILDROOT RPMS SOURCES SPECS SRPMS
cp /workspace/ice/packaging/rpm/ice.spec "$RPM_BUILD_ROOT/SPECS"

cd "$RPM_BUILD_ROOT/SOURCES"
spectool -g "$RPM_BUILD_ROOT/SPECS/ice.spec"

dnf builddep --spec "$RPM_BUILD_ROOT/SPECS/ice.spec" -y --setopt=multilib_policy=all

# Check if BUILD_ARCH is set and call setarch if so
if [[ -n "${BUILD_ARCH:-}" ]]; then
    setarch "$BUILD_ARCH"
fi

rpmbuild -bs "$RPM_BUILD_ROOT/SPECS/ice.spec" --define "_topdir $RPM_BUILD_ROOT"
rpmbuild -bb "$RPM_BUILD_ROOT/SPECS/ice.spec" --define "_topdir $RPM_BUILD_ROOT"
