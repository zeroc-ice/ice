#!/bin/bash
set -eux  # Exit on error, print commands

# Remove matching files recursively
find . -name '*.pem'    -exec rm -fv {} +
find . -name '*.der'    -exec rm -fv {} +
find . -name '*.p12'    -exec rm -fv {} +
find . -name '*.jks'    -exec rm -fv {} +
find . -name '*.old'    -exec rm -fv {} +
find . -name '*.csr'    -exec rm -fv {} +
find . -name '*.attr'    -exec rm -fv {} +
find . -name 'crlnumber' -exec rm -fv {} +
find . -name 'index.txt' -exec rm -fv {} +
find . -name 'serial'    -exec rm -fv {} +
find . -name 'newcerts'  -type d -exec rm -rfv {} +
find . -name 'crl'  -type d -exec rm -rfv {} +
