#!/bin/bash
set -eu -o pipefail

# Validate required environment variables
: "${ICE_NIGHTLY_PUBLISH_TOKEN:?Error: ICE_NIGHTLY_PUBLISH_TOKEN is not set}"
: "${STAGING_DIR:?Error: STAGING_DIR is not set}"
: "${CHANNEL:?Error: CHANNEL is not set}"
: "${QUALITY:?Error: QUALITY is not set}"

# Constants
indent="            " # 12 spaces for checksum line indentation

# Helper function to update Package.swift with URL and checksum for a zip file
# Arguments: $1=zip_file, $2=suffix (e.g., ".xcframework.zip"), $3=sed_pattern
update_package_for_zip() {
    local zip_file="$1" suffix="$2" sed_pattern="$3"
    local zip_name checksum zip_url

    zip_name=$(basename -s "${suffix}" "${zip_file}")
    zip_url="https://download.zeroc.com/${download_path}/${zip_name}${suffix}"
    checksum=$(shasum -a 256 "${zip_file}" | cut -d ' ' -f 1)

    # Perform sed replacement and verify it matched
    local before_checksum after_checksum
    before_checksum=$(shasum -a 256 Package.swift | cut -d ' ' -f 1)
    sed -i '' -e "s|${sed_pattern}|url: \"${zip_url}\",\n${indent}checksum: \"${checksum}\"|" Package.swift
    after_checksum=$(shasum -a 256 Package.swift | cut -d ' ' -f 1)

    if [ "$before_checksum" = "$after_checksum" ]; then
        echo "Error: sed pattern did not match for ${zip_name}${suffix}" >&2
        echo "Pattern: ${sed_pattern}" >&2
        exit 1
    fi
}

# Get Git repository root directory
root_dir=$(git rev-parse --show-toplevel)
cd "$root_dir/packaging/swift"

# Clone the ice-swift-nightly repository.
rm -rf ice-swift-nightly
git clone "https://x-access-token:${ICE_NIGHTLY_PUBLISH_TOKEN}@github.com/zeroc-ice/ice-swift-nightly.git" -b "${CHANNEL}"
cd ice-swift-nightly

# Remove existing directories to avoid keeping stale files that may have been removed from the Ice repository.
rm -rf cpp slice swift

# Copy the Ice for Swift sources and package manifest.
cp -rf ../../../cpp .
cp -rf ../../../slice .
cp -rf ../../../swift .
cp -f ../../../Package.swift .

# Compute the download path based on quality (matching publish-cpp-swift-deps.yml)
if [ "${QUALITY}" = "stable" ]; then
    download_path="ice/${CHANNEL}"
else
    download_path="ice/${QUALITY}/${CHANNEL}"
fi

# Collect xcframework and artifactbundle zip files from staging directory
shopt -s nullglob
xcframework_zips=("${STAGING_DIR}"/*.xcframework.zip)
artifactbundle_zips=("${STAGING_DIR}"/*.artifactbundle.zip)
shopt -u nullglob

# Validate that we found at least one zip file
if [ ${#xcframework_zips[@]} -eq 0 ] && [ ${#artifactbundle_zips[@]} -eq 0 ]; then
    echo "Error: No xcframework or artifactbundle zip files found in ${STAGING_DIR}" >&2
    exit 1
fi

# Extract and validate version from zip files (all must have the same version)
version=""
for zip_file in "${xcframework_zips[@]}" "${artifactbundle_zips[@]}"; do
    zip_name=$(basename "${zip_file}")
    # Remove both possible suffixes and extract version (everything after first hyphen)
    zip_name="${zip_name%.xcframework.zip}"
    zip_name="${zip_name%.artifactbundle.zip}"
    file_version="${zip_name#*-}"

    if [ -z "$version" ]; then
        version="$file_version"
    elif [ "$version" != "$file_version" ]; then
        echo "Error: Version mismatch - expected ${version}, found ${file_version} in $(basename "${zip_file}")" >&2
        exit 1
    fi
done

echo "Processing version: ${version}"

# Update the Package.swift file with the URL and Checksum for the xcframeworks
for zip_file in "${xcframework_zips[@]}"; do
    echo "Processing XCFramework zip file: ${zip_file}"
    zip_name=$(basename -s ".xcframework.zip" "${zip_file}")
    name="${zip_name%%-*}"
    update_package_for_zip "${zip_file}" ".xcframework.zip" "path: \".*${name}\\.xcframework\""
done

# Update the Package.swift file with the URL and Checksum for the slice2swift artifact bundle
for zip_file in "${artifactbundle_zips[@]}"; do
    echo "Processing artifact bundle zip file: ${zip_file}"
    update_package_for_zip "${zip_file}" ".artifactbundle.zip" "path: \"cpp/bin/slice2swift\\.artifactbundle\\.zip\""
done

# Commit and push the changes
git add .
git config user.name "ZeroC"
git config user.email "git@zeroc.com"
if ! git diff --cached --quiet; then
    git commit -m "ice: ${version} ${QUALITY} build"
    git tag -a "${version}" -m "ice: ${version} ${QUALITY} build"
    git push origin ${CHANNEL} --tags
else
    echo "No changes to commit"
fi
