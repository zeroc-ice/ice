#!/bin/bash
set -eux  # Exit on error, print commands

# Regenerate the Ice test certificates. Run via makecerts-docker.sh (recommended) or directly on a
# host that has openssl, keytool, the Bouncy Castle provider, and faketime installed.
#
# Usage: ./makecerts.sh [common|configuration]
#   no argument   regenerate everything
#   common        regenerate only the certs/ root certificates (the --protocol ssl certs)
#   configuration regenerate only the certs/configuration certificates (the IceSSL/configuration test)

# Default validity in days. 398 is the maximum accepted by macOS.
DEFAULT_DAYS=398

# Default password for the PKCS12, JKS and BKS files.
DEFAULT_PASSWORD="password"

# -legacy and the explicit PBE algorithms are required for compatibility with the macOS Keychain,
# which does not support the OpenSSL 3.0 PKCS12 defaults.
COMMON_PKCS12_ARGS=(-legacy -keypbe PBE-SHA1-3DES -certpbe PBE-SHA1-3DES -macalg sha1)

# The Bouncy Castle provider, used by keytool to create the Android .bks keystores.
BC_PROVIDER=org.bouncycastle.jce.provider.BouncyCastleProvider
BC_PROVIDER_PATH=$(ls /usr/share/java/bcprov*.jar 2>/dev/null | head -n1 || true)

# Export a leaf PKCS12 (key + cert chain) as a JKS keystore holding the leaf under "${alias}" and
# the CA certificate as a trusted entry under "cacert".
export_jks() {
    local p12="$1" jks="$2" alias="$3" ca_cert="$4"
    rm -f "${jks}"
    keytool -importkeystore \
        -srckeystore "${p12}" -srcstoretype PKCS12 -srcstorepass "${DEFAULT_PASSWORD}" \
        -destkeystore "${jks}" -deststoretype JKS -deststorepass "${DEFAULT_PASSWORD}" \
        -srcalias "${alias}" -destalias "${alias}"
    keytool -importcert -noprompt -alias cacert -file "${ca_cert}" \
        -keystore "${jks}" -storetype JKS -storepass "${DEFAULT_PASSWORD}"
}

# Export a leaf PKCS12 as a Bouncy Castle (.bks) keystore for the Android test controller.
export_bks() {
    local p12="$1" bks="$2" alias="$3" ca_cert="$4"
    [ -n "${BC_PROVIDER_PATH}" ] || { echo "Bouncy Castle provider not found; cannot create ${bks}"; return 1; }
    rm -f "${bks}"
    keytool -importkeystore \
        -srckeystore "${p12}" -srcstoretype PKCS12 -srcstorepass "${DEFAULT_PASSWORD}" \
        -destkeystore "${bks}" -deststoretype BKS -deststorepass "${DEFAULT_PASSWORD}" \
        -srcalias "${alias}" -destalias "${alias}" \
        -providerclass "${BC_PROVIDER}" -providerpath "${BC_PROVIDER_PATH}"
    keytool -importcert -noprompt -alias cacert -file "${ca_cert}" \
        -keystore "${bks}" -storetype BKS -storepass "${DEFAULT_PASSWORD}" \
        -providerclass "${BC_PROVIDER}" -providerpath "${BC_PROVIDER_PATH}"
}

# Generate the common CA, server and client certificates used to run the test suite with the
# -protocol ssl option. These live at the certs/ root with the flat 3.7 file names (cacert.pem,
# server.p12, ...) so the test driver references do not change.
generate_common() {
    # Self-signed root CA.
    openssl req -x509 -noenc -days "${DEFAULT_DAYS}" \
        -keyout ca_key.pem -out cacert.pem -config ca.cnf
    openssl x509 -in cacert.pem -out cacert.der -outform DER

    # Server and client certificates signed by the CA.
    local entity
    for entity in server client; do
        openssl req -new -noenc -keyout "${entity}_key.pem" -out "${entity}.csr" -config "${entity}.cnf"
        openssl x509 -req -in "${entity}.csr" \
            -CA cacert.pem -CAkey ca_key.pem -CAcreateserial \
            -days "${DEFAULT_DAYS}" -extfile "${entity}.cnf" -extensions v3_extensions \
            -out "${entity}.pem"
        openssl pkcs12 -export -out "${entity}.p12" \
            -inkey "${entity}_key.pem" -in "${entity}.pem" -certfile cacert.pem \
            -name "${entity}" -passout pass:"${DEFAULT_PASSWORD}" "${COMMON_PKCS12_ARGS[@]}"
        export_jks "${entity}.p12" "${entity}.jks" "${entity}" cacert.pem
        export_bks "${entity}.p12" "${entity}.bks" "${entity}" cacert.pem
    done

    # server.pem is the server certificate only (no key); it is consumed by scripts/Controller.py
    # and js/bin/HttpServer.mjs. The export above already wrote it as the signed certificate.

    # Remove intermediate artifacts we do not commit.
    rm -f ./*.csr ca_key.pem server_key.pem client_key.pem cacert.srl
}

# shellcheck source=makeconfiguration.sh
source "$(dirname "$0")/makeconfiguration.sh"

case "${1:-all}" in
    common)        generate_common ;;
    configuration) generate_configuration ;;
    all)           generate_common; generate_configuration ;;
    *)             echo "usage: $0 [common|configuration]"; exit 2 ;;
esac
