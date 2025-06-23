#!/usr/bin/env bash
set -euo pipefail

# Usage: ./create-repo.sh --distro <distro> --channel <channel> --staging <staging-dir> --repo <repo-dir>
# Required environment: GPG_KEY, GPG_KEY_ID

# Default values
DISTRO=""
CHANNEL=""
STAGING=""
REPODIR=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --distro)
            DISTRO="$2"
            shift 2
            ;;
        --channel)
            CHANNEL="$2"
            shift 2
            ;;
        --staging)
            STAGING="$2"
            shift 2
            ;;
        --repo)
            REPODIR="$2"
            shift 2
            ;;
        *)
            echo "Unknown argument: $1"
            exit 1
            ;;
    esac
done

# Validate required inputs
: "${DISTRO:?Missing --distro}"
: "${CHANNEL:?Missing --channel}"
: "${STAGING:?Missing --staging}"
: "${REPODIR:?Missing --repo}"
: "${GPG_KEY:?GPG_KEY environment variable is not set}"
: "${GPG_KEY_ID:?GPG_KEY_ID environment variable is not set}"

# Validate distro
case "$DISTRO" in
    debian12|ubuntu-24.04) ;;
    *)
        echo "Error: DISTRO must be 'debian12' or 'ubuntu-24.04'" >&2
        exit 1
        ;;
esac

# Validate channel
case "$CHANNEL" in
    3.8|nightly) ;;
    *)
        echo "Error: CHANNEL must be '3.8' or 'nightly'" >&2
        exit 1
        ;;
esac

# Import GPG key if not present
if ! gpg --list-secret-keys "$GPG_KEY_ID" > /dev/null 2>&1; then
    echo "$GPG_KEY" | gpg --batch --import
fi

declare -A CODENAMES=(
    ["debian12"]="bookworm"
    ["ubuntu-24.04"]="noble"
)

CODENAME="${CODENAMES[$DISTRO]}"
DIST_DIR="$REPODIR/$DISTRO"
CONF_DIR="$DIST_DIR/conf"

echo "Creating APT repository layout for $DISTRO in $DIST_DIR..."

mkdir -p "$CONF_DIR"

# Write conf/options
cat > "$CONF_DIR/options" <<EOF
verbose
basedir .
EOF

# Write conf/distributions
cat > "$CONF_DIR/distributions" <<EOF
Origin: ZeroC
Label: Ice $CHANNEL
Codename: $CODENAME
Suite: stable
Version: $CHANNEL
Architectures: amd64 arm64 source
Components: main
Description: ZeroC Ice $CHANNEL packages for $DISTRO
SignWith: $GPG_KEY_ID
EOF

echo "✓ Repository config created at $CONF_DIR"

# Cleanup old nightly packages
if [[ "$CHANNEL" == "nightly" ]]; then
    DAYS_TO_KEEP=3
    today_sec=$(date +%s)

    reprepro -b "$DIST_DIR" list "$CODENAME" | while read -r line; do
        arch=$(echo $line | awk -F'[|: ]+' '{print $3}')
        pkg=$(echo $line | awk -F'[|: ]+' '{print $4}')
        version=$(echo $line | awk -F'[|: ]+' '{print $5}')
        if [[ "$version" =~ nightly([0-9]{8}) ]]; then
            date_part="${BASH_REMATCH[1]}"
            pkg_date_sec=$(date -d "$date_part" +%s || true)

            if [[ -z "$pkg_date_sec" || -z "$today_sec" ]]; then
                echo "⚠️ Skipping $pkg (invalid date format: $date_part)"
                continue
            fi

            age_days=$(( (today_sec - pkg_date_sec) / 86400 ))
            if (( age_days > DAYS_TO_KEEP )); then
                echo "🗑️ Removing $pkg version $version (arch: $arch, age: ${age_days} days)"
                reprepro -b "$DIST_DIR" remove "$CODENAME" "$pkg"
            fi
        fi
    done
fi

# Collect binary packages
packages=()
while IFS= read -r file;
    do packages+=("$file");
done < <(find "$STAGING/deb-packages-$DISTRO-x86_64" -type f -name "*_amd64.deb")

while IFS= read -r file; do
    packages+=("$file");
done < <(find "$STAGING/deb-packages-$DISTRO-aarch64" -type f -name "*_arm64.deb")

while IFS= read -r file; do
    packages+=("$file");
done < <(find "$STAGING/deb-packages-$DISTRO-x86_64" -type f -name "*_all.deb")

# Add binary packages
for package in "${packages[@]}"; do
    reprepro -b "$DIST_DIR" includedeb "$CODENAME" "$package"
done

# Collect source packages
src_packages=()
while IFS= read -r file; do
    src_packages+=("$file");
done < <(find "$STAGING/deb-packages-$DISTRO-x86_64" -type f -name "*.dsc")

# Add source packages
for package in "${src_packages[@]}"; do
    reprepro -b "$DIST_DIR" includedsc "$CODENAME" "$package"
done
