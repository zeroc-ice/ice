#!/bin/bash
set -eux  # Exit on error, print commands

# Ensure ICE_VERSION is set
if [ -z "$ICE_VERSION" ]; then
    echo "Error: ICE_VERSION is not set!"
    exit 1
fi

# Generate the upstream tarball
echo "Creating tarball for ICE_VERSION=$ICE_VERSION"
tar -czf /workspace/build/zeroc-ice_${ICE_VERSION}.orig.tar.gz --exclude=debian --exclude=.build -C /workspace/ice .

# Create build directory and unpack
mkdir -p /workspace/build/ice
cd /workspace/build/ice
tar xzf ../zeroc-ice_${ICE_VERSION}.orig.tar.gz

# Copy Debian packaging files and build
cp -rfv /workspace/ice/packaging/dpkg/debian .
dpkg-buildpackage -S
dpkg-buildpackage -b -uc -us
