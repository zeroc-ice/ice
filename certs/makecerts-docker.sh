#!/bin/bash
# Regenerate the Ice test certificates inside a Docker container, so the host only needs Docker —
# not openssl, keytool, the Bouncy Castle provider, and faketime. See makecerts.sh for the actual
# certificate generation.
set -eux

# Directory containing this script (the certs/ directory).
certs_dir=$(cd "$(dirname "$0")" && pwd)

# Build the image with openssl, keytool (JDK), the Bouncy Castle provider, and faketime.
docker build -t ice-makecerts "${certs_dir}"

# Run makecerts.sh inside the container, mounting certs/ so the regenerated files are written back
# to the host. Map the container user to the host user so the generated files are owned by you
# rather than root. HOME is set to a writable location because the mapped user has no entry in the
# image's /etc/passwd.
docker run --rm \
    -u "$(id -u):$(id -g)" \
    -e HOME=/tmp \
    -v "${certs_dir}":/certs \
    ice-makecerts "$@"
