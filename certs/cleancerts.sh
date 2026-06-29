#!/bin/bash
set -eux  # Exit on error, print commands

# Remove all generated certificate material so makecerts.sh can regenerate it from scratch. This
# keeps the inputs: the .cnf files at the certs/ root, and the custom-extension request/extension
# files and the RNG seed (cacert_custom.req, cacert_custom.ext, seed.dat) under certs/configuration.

# Common certificates at the certs/ root.
find . -maxdepth 1 -type f \( \
    -name '*.pem' -o -name '*.der' -o -name '*.p12' -o -name '*.jks' -o -name '*.bks' \
    -o -name '*.csr' -o -name '*.srl' \
\) -exec rm -fv {} +

# Configuration certificates (the openssl CA databases plus the generated certificates).
rm -rf configuration/db
find configuration -maxdepth 1 -type f \( \
    -name '*.pem' -o -name '*.der' -o -name '*.p12' -o -name '*.jks' -o -name '*.csr' \
\) -exec rm -fv {} +
