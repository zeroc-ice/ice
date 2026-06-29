# Image for regenerating the Ice test certificates. It bundles the tools used by makecerts.sh —
# openssl (with legacy PBE support for the macOS Keychain), keytool (from the JDK, for the .jks
# keystores), the Bouncy Castle provider (for the Android .bks keystores), and faketime (for the
# expired/not-yet-valid fixtures) — so the host only needs Docker.
FROM debian:12-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
        openssl \
        default-jdk-headless \
        libbcprov-java \
        faketime \
        ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /certs

# makecerts.sh is mounted at runtime via the certs/ volume (see makecerts-docker.sh).
ENTRYPOINT ["./makecerts.sh"]
