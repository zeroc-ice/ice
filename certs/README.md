# Ice Test Certificates

This directory contains the X.509 certificates and private keys used to test the SSL transport.
These certificates are intended for **testing purposes only** and **must not** be used in a
production environment.

## Layout

| Path | Used by |
|------|---------|
| `certs/` (root: `cacert.pem`, `server.p12`, `client.p12`, `server.pem`, `*.jks`, `*.bks`, …) | Running the test suite with `--protocol ssl` (the "common" certificates). |
| `certs/configuration/` | The `IceSSL/configuration` test in every language mapping (C++, C#, Java, java-compat, Swift). |

The `configuration` certificates were previously duplicated under `cpp/test/IceSSL/certs`,
`csharp/test/IceSSL/certs`, `java/test/.../IceSSL/certs`, and `java-compat/test/.../IceSSL/certs`.
They are now stored once, here.

The common keystores come in several formats so every mapping can use them: `*.p12` (PKCS12, for
C++ and C#), `*.jks` (Java), and `*.bks` (Bouncy Castle, for the Android test controller).

## Regenerating the Certificates

The certificates are valid for 398 days (the maximum accepted by macOS), so they must be
regenerated roughly once a year.

`makecerts.sh` currently regenerates the **common** certificates (at the `certs/` root). The
`configuration` certificates are consolidated in `certs/configuration` but are still the set
generated previously with `zeroc-icecertutils`; porting their generation to this openssl/keytool
toolchain is tracked separately.

> **Note:** regenerating creates new key pairs, which changes key-derived values (subject key
> identifiers, authority key identifiers, and SHA-1 thumbprints). When the `configuration`
> certificates are regenerated, the corresponding literals in
> `cpp/test/IceSSL/configuration/AllTests.cpp` and `csharp/test/IceSSL/configuration/AllTests.cs`
> must be updated. The certificate subjects, serial numbers (`s_rsa_ca1` = 1, `c_rsa_ca1` = 2),
> SANs and extended key usages should be reproduced so the DN-based assertions do not change.

### Using Docker (recommended)

Run `makecerts-docker.sh` to regenerate the common certificates inside a container that bundles the
required tools, so the host only needs Docker:

```shell
./makecerts-docker.sh          # regenerate the common (--protocol ssl) certificates
```

This builds the `Dockerfile` in this directory (with `openssl`, `keytool`, the Bouncy Castle
provider, and `faketime`) and runs `makecerts.sh` inside it.

### Without Docker

Run `makecerts.sh` directly. This requires `openssl`, `keytool`, and the Bouncy Castle provider to
be installed on the host. (The Docker image also bundles `faketime`, which the configuration
certificates will need once their generation is ported to this toolchain.)

## Contents of `configuration`

* `ca1` the main CA used for certificate verification, plus its intermediates (`cai1`, `cai2`),
  the hostname/SAN matrix (`s_rsa_ca1_cn1`…`cn11`), the extended-key-usage matrix (`rsa_ca1_*`),
  the with-root and password-protected variants, and expired certificates.
* `ca2` an alternate, self-signed CA used in the multi-CA test (also used as a server certificate).
* `ca3` certificate revocation list (CRL) tests, plus its intermediate `cai3`.
* `ca4` OCSP revocation tests, plus its intermediate `cai4`.
* `cacert_custom` a certificate carrying custom X.509 extensions.
* `dh_params512.der` / `dh_params1024.der` Diffie-Hellman parameters.
* `password_less_*` PKCS12 files protected with an empty password (C# test).
