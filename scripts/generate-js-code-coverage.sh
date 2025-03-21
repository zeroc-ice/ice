#!/bin/sh
set -eux

# Change to the js/ directory relative to this script's location
cd "$(dirname "$0")/../js"

# Remove any previous raw coverage output
rm -rf coverage

# Run the full test suite with coverage
python3 allTests.py --coverage --debug

# Prepare the aggregated output directory
output_dir="coverage-report"
rm -rf "$output_dir"
mkdir -p "$output_dir/tmp"

# Find and rename all per-process coverage files
find coverage -type f -name 'coverage-*.json' | while read -r json_file; do
    echo "Processing $json_file"

    relpath="${json_file#coverage/}"                      # Strip leading "coverage/"
    testdir="${relpath%%/tmp/*}"                          # Extract test subdir (e.g. Ice/foo-Client.js)
    filename=$(basename "$json_file")                     # Just the JSON filename

    newname="${testdir//\//-}-$filename"                  # Replace / with - for flat naming
    cp "$json_file" "$output_dir/tmp/$newname"
done

# Generate final merged HTML report from all renamed JSON files
npx c8 report --report-dir="$output_dir" --reporter=html --reporter=text-summary

# Clean up the temp .json files after reporting
rm -rf "$output_dir/tmp"
