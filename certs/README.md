# Ice Test Certificates

This directory contains the X.509 certificates and private keys used to test the SSL transport. These
certificates are intended for **testing purposes only** and **must not** be used in a production environment.

## Regenerating the Certificates

Use the `makecerts.sh` script to generate new test certificates. This script uses the `openssl` and `keytool`
command-line tools to recreate all keys and certificates.

If you plan to run the ssl transport tests across multiple machines, you may need to update the `alt_names` section
in `server.cnf` to match your environment.
