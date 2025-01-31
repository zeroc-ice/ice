#!/bin/bash
set -eux  # Exit on error, print commands

# Ensure ICE_VERSION is set
if [ -z "$ICE_VERSION" ]; then
    echo "Error: ICE_VERSION is not set!"
    exit 1
fi

# Set working directory
cd /workspace

# Generate the upstream tarball
echo "Creating tarball for ICE_VERSION=$ICE_VERSION"
tar -czf zeroc-ice_${ICE_VERSION}.orig.tar.gz --exclude=debian -C /workspace/ice .

# Create build directory and unpack
mkdir -p build
cd build
tar xzf ../zeroc-ice_${ICE_VERSION}.orig.tar.gz

# Copy Debian packaging files and build
cp -rfv ../ice/packaging/dpkg/debian .
dpkg-buildpackage -S
dpkg-buildpackage -b -uc -us
