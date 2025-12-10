#!/usr/bin/env bash
set -euo pipefail

# Config (override via env or CLI if you like)
BUCKET="${BUCKET:-zeroc-downloads}"
PREFIX="${PREFIX:-ice/nightly/}"   # no leading slash
DAYS_TO_KEEP="${DAYS_TO_KEEP:-7}"  # default: keep last 7 days
DRY_RUN="${DRY_RUN:-1}"            # set to 0 to actually delete

echo "Bucket       : $BUCKET"
echo "Prefix       : $PREFIX"
echo "Days to keep : $DAYS_TO_KEEP"
echo "Dry run      : $DRY_RUN"

today_sec=$(date +%s)
deleted=0
kept=0
ignored=0

# Get all keys in one go. If this fails, we skip prune but don't kill CI.
if ! keys=$(aws s3api list-objects-v2 \
        --bucket "$BUCKET" \
        --prefix "$PREFIX" \
        --query 'Contents[].Key' \
        --output text 2>aws-list-error.log); then
    echo "⚠️  Failed to list S3 objects for pruning, skipping prune (non-fatal)" >&2
    cat aws-list-error.log >&2 || true
    echo "Finished S3 nightly prune:"
    echo "  Deleted : 0"
    echo "  Kept    : 0"
    echo "  Ignored : 0"
    exit 0
fi

# Iterate over keys (split on whitespace). This avoids any pipe/`tr` → no broken pipe.
for key in $keys; do
    # empty guard (just in case)
    [[ -z "$key" ]] && continue

    # ignore "directories"
    if [[ "$key" == */ ]]; then
        echo "📁 Ignoring directory-like key: $key"
        ((ignored++))
        continue
    fi

    # Extract nightly date part: nightly.20250821
    if [[ "$key" =~ nightly[.-]?([0-9]{8}) ]]; then
        date_part="${BASH_REMATCH[1]}"

        echo "DEBUG: date binary: $(command -v date)"
        date --version || date

        echo "DEBUG: raw key: [$key]"
        printf 'DEBUG: date_part bytes: ' >&2
        printf '%s' "$date_part" | hexdump -C >&2 || true

        echo "DEBUG: running: date -d \"$date_part\" +%s" >&2
        pkg_date_sec=$(date -d "$date_part" +%s 2>&1)
        echo "DEBUG: date output: [$pkg_date_sec]" >&2

        # Convert YYYYMMDD to epoch seconds (GNU date); handle failure
        pkg_date_sec=$(date -d "$date_part" +%s 2>/dev/null || echo 0)
        if (( pkg_date_sec <= 0 )); then
            echo "⚠️  Skipping $key (invalid date: $date_part)"
            ((ignored++))
            continue
        fi

        # Don't delete “future” objects if clock skew or typo
        if (( pkg_date_sec > today_sec )); then
            echo "⚠️  Skipping $key (date $date_part is in the future)"
            ((ignored++))
            continue
        fi

        age_days=$(( (today_sec - pkg_date_sec) / 86400 ))

        if (( age_days > DAYS_TO_KEEP )); then
            echo "🧹 Deleting (age ${age_days}d): s3://$BUCKET/$key"
            if [[ "$DRY_RUN" != "1" ]]; then
                # Treat aws s3 rm failures as non-fatal: log and continue
                if ! aws s3 rm "s3://$BUCKET/$key"; then
                    echo "⚠️  Failed to delete s3://$BUCKET/$key, continuing" >&2
                    ((ignored++))
                    continue
                fi
            fi
            ((deleted++))
        else
            #echo "✅ Keeping (age ${age_days}d): s3://$BUCKET/$key"
            ((kept++))
        fi
    else
        # No nightly.YYYYMMDD → ignore
        #echo "ℹ️  Ignoring (no nightly.YYYYMMDD date part): $key"
        ((ignored++))
    fi
done

echo "Finished S3 nightly prune:"
echo "  Deleted : $deleted"
echo "  Kept    : $kept"
echo "  Ignored : $ignored"
