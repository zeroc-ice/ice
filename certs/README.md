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
tools to be installed on the host, plus Python 3 with the
[cryptography](https://pypi.org/project/cryptography/) package:

```shell
./makecerts.sh
```

## Password-less PKCS#12 files

RFC 7292 derives the MAC and encryption keys from a null-terminated UTF-16BE password, but leaves the empty
password ambiguous: it is either a zero-length byte string, or a zero-length BMPString, which is just the two
`0x00` terminator bytes. The two derive different keys, so a file written under one reading fails MAC
verification under the other. Readers are generally lenient and accept both; writers pick one.

`openssl`, `keytool`, and python-`cryptography` all write the two-byte form. Apple's Security framework accepts
only the zero-length form. That is why there are two sets of password-less fixtures under
`configuration/ca1`:

| File | Empty password encoding | Loads on |
| --- | --- | --- |
| `client_password_less.p12`, `server_password_less.p12` | zero-length BMPString (`openssl -passout pass:`) | every platform except macOS and iOS |
| `client_null_password.p12`, `server_null_password.p12` | zero-length byte string (`make_passwordless_pkcs12.py`) | every platform |

`make_passwordless_pkcs12.py` exists because `openssl` cannot write the zero-length form. It also explains why
the private key is stored in a `pkcs8ShroudedKeyBag` rather than a plain one, and why PBE-SHA1-3DES is used
instead of the PBES2/AES defaults.

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
