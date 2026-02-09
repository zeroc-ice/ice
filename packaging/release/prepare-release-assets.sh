#!/usr/bin/env bash
set -euxo pipefail

mkdir -p release
pushd release

# Create DEB package archives (.tar.gz) for each distribution/arch
for dir in "${STAGING_DIR}"/deb-packages-*; do
  name=$(basename "$dir")
  mkdir -p "$name"
  cp -v "$dir"/* "$name/"
  tar -czvf "${name}.tar.gz" "$name"
  rm -rf "$name"
done

# Bundle .NET NuGet packages into zip and tar.gz archives
mkdir -p dotnet-nuget-packages
find "${STAGING_DIR}/dotnet-nuget-packages" -type f \( -name '*.nupkg' -o -name '*.snupkg' \) \
  -exec cp -v {} dotnet-nuget-packages/ \;
zip -r dotnet-nuget-packages.zip dotnet-nuget-packages
tar -czvf dotnet-nuget-packages.tar.gz dotnet-nuget-packages
rm -rf dotnet-nuget-packages

# Include Ruby GEM packages as is.
cp -v "${STAGING_DIR}/gem-packages"/*.gem .

# Include C++ Dependencies for Swift (XCFrameworks and slice2swift artifact bundle).
cp -v "${STAGING_DIR}/cpp-swift-deps"/*.zip .

# Include MATLAB toolbox packages (.mltbx).
cp -v "${STAGING_DIR}"/matlab-packages-*/*.mltbx .

# Include Windows MSI installers.
cp -v "${STAGING_DIR}/windows-msi"/*.msi .

# Create Java package archives (.zip and .tar.gz) with all Java artifacts.
mkdir -p java-packages/{lib,tools}
cp -v "${STAGING_DIR}/java-packages"/* java-packages/lib/
cp -vr "${STAGING_DIR}/slice-tools-packages"/* java-packages/tools/

zip -r java-packages.zip java-packages
tar -czf java-packages.tar.gz java-packages

rm -rf java-packages

# Include JavaScript NPM package.
cp -v "${STAGING_DIR}/js-npm-packages"/*.tgz .

# Include Python wheels and PIP source distributions.
for dir in "${STAGING_DIR}"/pip-packages-*; do
  cp -v "$dir"/zeroc_ice-*.whl .
done
cp -v "${STAGING_DIR}/pip-sdist"/zeroc_ice-*.tar.gz .

# IceGridGUI JAR package
cp -v "${STAGING_DIR}/icegridgui-jar/icegridgui.jar" .

# Create RPM package archives (.tar.gz) for each distribution/arch
for dir in "${STAGING_DIR}"/rpm-packages-*; do
  name=$(basename "$dir")
  mkdir -p "$name"
  cp -rv "$dir"/* "$name/"
  tar -czf "${name}.tar.gz" "$name"
  rm -rf "$name"
done

# Create Windows symbols and sources archive (.zip)
zip -r "windows-symbols-sources.zip" "${STAGING_DIR}/windows-symbols-sources"

popd
