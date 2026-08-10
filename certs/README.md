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

RFC 7292 gives two incompatible answers for an empty password. Appendix B.1 says every password is a BMPString
whose last character is "followed by 2 additional bytes with the value `0x00`", which makes the empty password
two `0x00` bytes. Appendix B.2 step 3, deriving the key, instead notes that "if the password is the empty
string, then so is P" — a zero-length block. (Read strictly, B.1 is the formatting step and B.2 works on its
output, so B.1 wins; implementations disagree anyway.) The two derive different keys, so a file written under
one reading fails MAC verification under the other. Writers pick one, and so do some readers.

`openssl`, `keytool`, and python-`cryptography` all write the two-byte form. OpenSSL, Schannel, Java and .NET
read both. The two Apple platforms each accept only one form, and they do not agree on which: macOS reads only
the zero-length form, iOS only the two-byte form. Each rejects the other with `errSecAuthFailed` (-25293). That
is why there are two sets of password-less fixtures under `configuration/ca1`:

| File | Empty password encoding | Loads on |
| --- | --- | --- |
| `client_password_less.p12`, `server_password_less.p12` | two `0x00` bytes, RFC 7292 B.1 (`openssl -passout pass:`) | every platform except macOS |
| `client_null_password.p12`, `server_null_password.p12` | zero-length block, RFC 7292 B.2 step 3 (`make_passwordless_pkcs12.py`) | every platform except iOS |

There is therefore no single password-less PKCS#12 file that loads on both macOS and iOS, and the IceSSL
configuration test picks the fixture that matches the platform it runs on.

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
