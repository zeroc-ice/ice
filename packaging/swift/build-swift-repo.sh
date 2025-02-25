#!/bin/bash
set -eux -o pipefail

usage() {
    echo "Usage: $0 <ice_version> <repository_url>"
}

ice_version=${1:-}

if [ -z "$ice_version" ]; then
    usage
    exit 1
fi

repository_url=${2:-}

if [ -z "$repository_url" ]; then
    usage
    exit 1
fi

# Get Git repository root directory
root_dir=$(git rev-parse --show-toplevel)
cd "$root_dir"/packaging/swift

for name in Ice IceDiscovery IceLocatorDiscovery; do
    zip_name=$name-$ice_version.xcframework.zip
    curl -fsSL -o "${zip_name}" "$repository_url/$zip_name"
    echo "Computing SHA256 for ${zip_name}"
    declare "${name}_XCFRAMEWORK_CHECKSUM=$( shasum -a 256 "${zip_name}" | cut -d ' ' -f 1 )"
    export "${name}_XCFRAMEWORK_CHECKSUM"

    declare "${name}_XCFRAMEWORK_URL=$repository_url/$zip_name"
    export "${name}_XCFRAMEWORK_URL"
done

[ -d IceSwift ] && rm -rf IceSwift
mkdir IceSwift

envsubst < Package.swift > IceSwift/Package.swift

mkdir IceSwift/Sources

mkdir -p IceSwift/cpp/{src,include}/Ice

ice_util_files=(
    "ConsoleUtil.cpp"
    "CtrlCHandler.cpp"
    "Demangle.cpp"
    "Exception.cpp"
    "FileUtil.cpp"
    "LocalException.cpp"
    "Options.cpp"
    "OutputUtil.cpp"
    "Random.cpp"
    "StringConverter.cpp"
    "StringUtil.cpp"
    "UUID.cpp"
)

for source_file in "${ice_util_files[@]}"; do
    cp -fv "../../cpp/src/Ice/$source_file" IceSwift/cpp/src/Ice/
    # strip leading src/ and replace .cpp with .h
    header_file=${source_file%.cpp}.h

    [ -f "../../cpp/src/Ice/$header_file" ] && cp -rfv "../../cpp/src/Ice/$header_file" IceSwift/cpp/src/Ice/
    [ -f "../../cpp/include/Ice/$header_file" ] && cp -rfv "../../cpp/include/Ice/$header_file" IceSwift/cpp/include/Ice
done

# Extra headers required to build slice2swift
ice_extra_headers=(
    "include/Ice/Config.h"
    "src/Ice/ScannerConfig.h"
    "src/Ice/DisableWarnings.h"
)

for header_path in "${ice_extra_headers[@]}"; do
    cp -fv "../../cpp/$header_path" "IceSwift/cpp/${header_path}"
done

cp -fv ../../cpp/include/Ice/Config.h IceSwift/cpp/include/Ice

mkdir -p IceSwift/cpp/src/Slice
cp -rfv ../../cpp/src/Slice/*.{h,cpp} IceSwift/cpp/src/Slice/

mkdir -p IceSwift/cpp/src/slice2swift
cp -rfv ../../cpp/src/slice2swift/*.{h,cpp} IceSwift/cpp/src/slice2swift/

cp -rfv ../../swift/src/* IceSwift/Sources/
cp -rfv ../../swift/Plugins* IceSwift/Plugins

cp -rfv ../../slice IceSwift/slice

# Find all slice-plugin.json files and replace "../../../slice" with "../../slice"
find IceSwift -name slice-plugin.json -exec sed -i '' 's|../../../slice|../../slice|g' {} \;

cd IceSwift
git init .
git add .
git config user.name "ZeroC"
git config user.email "git@zeroc.com"
git commit -m "ice: $ice_version"
git tag -a "$ice_version" -m "ice: $ice_version"
