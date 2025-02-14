#!/bin/bash
set -eu

# Ensure the repository url is set
if [ -z "${REPOSITORY_URL}" ]; then
    echo "Error: REPOSITORY_URL must be set as environment variable."
    exit 1
fi

# Iterate over all .rpm files in the current directory and subdirectories
find . -type f -name "*.rpm" | while read -r file; do
    echo "Uploading $file to Nexus..."
    curl -u "${REPOSITORY_USERNAME}:${REPOSITORY_PASSWORD}" \
         -H "Content-Type: multipart/form-data" \
         --data-binary "@$file" \
         --output /dev/null \
         --silent \
         --fail \
         --show-error \
         "${REPOSITORY_URL}" || { echo "Upload failed: $file"; exit 1; }
    echo "Successfully uploaded: $file"
done
