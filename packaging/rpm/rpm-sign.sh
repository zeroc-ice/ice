#!/bin/bash
set -euo pipefail

# Ensure GPG_KEY and GPG_KEY_ID are set
: "${GPG_KEY:?GPG_KEY environment variable is not set}"
: "${GPG_KEY_ID:?GPG_KEY_ID environment variable is not set}"

# mutt GPG tty setting
export GPG_TTY=$(tty)

# Import the GPG key
echo "$GPG_KEY" | gpg --batch --import

# Check that the key was successfully imported
if ! gpg --list-secret-keys "$GPG_KEY_ID" > /dev/null 2>&1; then
  echo "Error: GPG key ID $GPG_KEY_ID was not imported successfully."
  exit 1
fi

# Set up ~/.rpmmacros for rpmsign
cat > ~/.rpmmacros <<EOF
%_signature gpg
%_gpg_name $GPG_KEY_ID
%_gpg_path ~/.gnupg
%__gpg_check_password_cmd /bin/true
%__gpg /usr/bin/gpg
EOF

# Directory containing the RPMs
BUILD_DIR="${1:?Usage: $0 <build-dir>}"

# Find all RPMs for the current arch
mapfile -t rpms < <(find "$BUILD_DIR" -type f -name "*.rpm")

for rpm in "${rpms[@]}"; do
  echo "Signing: $(basename "$rpm")"
  rpmsign --addsign "$rpm"
done
