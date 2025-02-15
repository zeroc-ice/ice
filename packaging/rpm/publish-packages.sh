#!/bin/bash
set -eu

# Ensure the repository url is set
if [ -z "${REPOSITORY_URL}" ]; then
    echo "Error: REPOSITORY_URL must be set as environment variable."
    exit 1
fi

# The rpmbuild command creates the package files in the RPMS directory.
cd /workspace/build/RPMS

# Iterate over all .rpm files in the current directory and subdirectories
find . -type f -name "*.rpm" | while read -r file; do
    echo "Uploading $file to Nexus..."
    curl -u "${REPOSITORY_USERNAME}:${REPOSITORY_PASSWORD}" \
         --upload-file "$file" \
         --output /dev/null \
         --silent \
         --fail \
         --show-error \
         "${REPOSITORY_URL}" || { echo "Upload failed: $file"; exit 1; }
    echo "Successfully uploaded: $file"
done
