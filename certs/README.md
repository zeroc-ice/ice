# Ice Test Certificates

This directory contains the X.509 certificates and private keys used to test the SSL transport. These
certificates are intended for **testing purposes only** and **must not** be used in a production environment.

## Regenerating the Certificates

The certificates are valid for 398 days (the maximum accepted by macOS), so they must be regenerated roughly
once a year.

### Using Docker (recommended)

Run `makecerts-docker.sh` to regenerate everything inside a container that bundles the required tools, so the
host only needs Docker:

```shell
./makecerts-docker.sh
```

This builds the `Dockerfile` in this directory (with `openssl`, `keytool`, and `faketime`) and runs
`makecerts.sh` inside it, writing the regenerated certificates back to this directory. The generated files are
owned by the invoking user.

### Without Docker

Run the `makecerts.sh` script directly. This requires the `openssl`, `keytool`, and `faketime` command-line
tools to be installed on the host:

```shell
./makecerts.sh
```

## Contents

### common/ca

The `common/ca` directory contains the certificates and keys used for running Ice tests with `-protocol ssl` option.

If you plan to run the tests across multiple machines, you may need to update the `alt_names` section in `server.cnf`
to match your environment.

### configuration

The `configuration` directory contains the certificates and keys used by IceSSL/configuration test.

* `configuration/ca1` Used for certificate verification
* `configuration/ca2` An alternate CA used in multi CA test
* `configuration/ca3` CRL revocation tests
* `configuration/ca4` OCSP revocation tests
* `configuration/ca5` Expired and not yet valid certificates.
