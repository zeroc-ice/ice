#!/usr/bin/env bash
set -euo pipefail

# Usage: ./create-deb-repo.sh --distribution <distribution> --channel <channel> --staging <staging-dir> --repo <repo-dir>
# Required environment: GPG_KEY, GPG_KEY_ID

# Default values
DISTRIBUTION=""
CHANNEL=""
STAGING=""
REPODIR=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --distribution)
            DISTRIBUTION="$2"
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
        --repository)
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
: "${DISTRIBUTION:?Missing --distribution}"
: "${CHANNEL:?Missing --channel}"
: "${STAGING:?Missing --staging}"
: "${REPODIR:?Missing --repo}"
: "${GPG_KEY:?GPG_KEY environment variable is not set}"
: "${GPG_KEY_ID:?GPG_KEY_ID environment variable is not set}"

# Validate distribution
case "$DISTRIBUTION" in
    debian12|ubuntu24.04) ;;
    *)
        echo "Error: DISTRIBUTION must be 'debian12' or 'ubuntu24.04'" >&2
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

# Import the GPG key
echo "$GPG_KEY" | gpg --batch --import

# Check that the key was successfully imported
if ! gpg --list-secret-keys "$GPG_KEY_ID" > /dev/null 2>&1; then
  echo "Error: GPG key ID $GPG_KEY_ID was not imported successfully."
  exit 1
fi

declare -A CODENAMES=(
    ["debian12"]="bookworm"
    ["ubuntu24.04"]="noble"
)

CODENAME="${CODENAMES[$DISTRIBUTION]}"
DIST_DIR="$REPODIR/$DISTRIBUTION"
CONF_DIR="$DIST_DIR/conf"

echo "Creating APT repository layout for $DISTRIBUTION in $DIST_DIR..."

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
Description: ZeroC Ice $CHANNEL packages for $DISTRIBUTION
SignWith: $GPG_KEY_ID
EOF

echo "✓ Repository config created at $CONF_DIR"

# Cleanup old nightly packages
if [[ "$CHANNEL" == "nightly" ]]; then
    DAYS_TO_KEEP=3
    today_sec=$(date +%s)
    declare -A seen_versions=()

    echo "🔍 Scanning for nightly versions older than $DAYS_TO_KEEP days..."

    while read -r line; do
        version=$(echo "$line" | awk -F'[|: ]+' '{print $5}')
        if [[ "$version" =~ nightly([0-9]{8}) ]]; then
            date_part="${BASH_REMATCH[1]}"
            pkg_date_sec=$(date -d "$date_part" +%s 2>/dev/null || echo 0)

            if (( pkg_date_sec <= 0 )); then
                echo "⚠️ Skipping version $version (invalid date: $date_part)"
                continue
            fi

            age_days=$(( (today_sec - pkg_date_sec) / 86400 ))
            if (( age_days > DAYS_TO_KEEP )); then
                if [[ -z "${seen_versions[$version]+_}" ]]; then
                    seen_versions["$version"]=1
                fi
            fi
        fi
    done < <(reprepro -b "$DIST_DIR" list "$CODENAME")

    if (( ${#seen_versions[@]} > 0 )); then
        echo "🧹 Removing ${#seen_versions[@]} outdated nightly version(s):"
        for version in "${!seen_versions[@]}"; do
            echo "   - $version"
            reprepro -b "$DIST_DIR" removefilter "$CODENAME" "Version (== $version)"
        done
    else
        echo "✅ No old nightly versions to prune."
    fi
fi

# Collect binary packages
packages=()
while IFS= read -r file;
    do packages+=("$file");
done < <(find "$STAGING/deb-packages-$DISTRIBUTION-amd64" -type f -name "*_amd64.deb")

while IFS= read -r file; do
    packages+=("$file");
done < <(find "$STAGING/deb-packages-$DISTRIBUTION-arm64" -type f -name "*_arm64.deb")

while IFS= read -r file; do
    packages+=("$file");
done < <(find "$STAGING/deb-packages-$DISTRIBUTION-amd64" -type f -name "*_all.deb")

# Add binary packages
for package in "${packages[@]}"; do
    reprepro -b "$DIST_DIR" includedeb "$CODENAME" "$package"
done

# Collect source packages
src_packages=()
while IFS= read -r file; do
    src_packages+=("$file");
done < <(find "$STAGING/deb-packages-$DISTRIBUTION-amd64" -type f -name "*.dsc")

# Add source packages
for package in "${src_packages[@]}"; do
    reprepro -b "$DIST_DIR" includedsc "$CODENAME" "$package"
done
