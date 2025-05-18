#!/bin/bash
set -e

# The password used for the PKCS#12 certificate stores.
PASSWORD="password"

# Certificate validity duration. Set to 398 days to comply with macOS requirements.
DAYS=398

# Create the Test CA private key and self-signed certificate
openssl req -x509 -nodes -days ${DAYS} -newkey rsa:2048 \
  -keyout ca_key.pem -out ca_cert.pem -config ca.cnf

rm -f ca.p12
keytool -importcert \
  -alias CA \
  -file ca_cert.pem \
  -keystore ca.p12 \
  -storetype PKCS12 \
  -storepass ${PASSWORD} \
  -noprompt

for name in server client; do
    echo "Generating certificate for ${name}..."

    # Generate key and CSR
    openssl req -new -nodes -out ${name}.csr -keyout ${name}_key.pem -config ${name}.cnf

    # Sign certificate with CA
    openssl x509 -req -in ${name}.csr -CA ca_cert.pem -CAkey ca_key.pem -CAcreateserial \
        -out ${name}_cert.pem -days ${DAYS} -sha256 -extfile ${name}.cnf -extensions v3_extensions

    # Remove CSR after signing
    rm -f ${name}.csr

    # Export to PKCS#12
    rm -f ${name}.p12
    openssl pkcs12 -export -out ${name}.p12 \
        -inkey ${name}_key.pem -in ${name}_cert.pem -certfile ca_cert.pem \
        -name "${name}" -passout pass:"${PASSWORD}" \
        -legacy # For compatibility with macOS Keychain
done

# We don't need the CA private key or serial file after certs are generated
rm -f ca_key.pem
rm -f ca_cert.srl
