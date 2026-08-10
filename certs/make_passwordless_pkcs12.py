#!/usr/bin/env python3
"""Write a PKCS#12 file with an empty password that macOS accepts.

RFC 7292 gives two incompatible answers for an empty password. Appendix B.1 says every password is a BMPString
whose last character is "followed by 2 additional bytes with the value 0x00", which makes the empty password two
0x00 bytes. Appendix B.2 step 3, deriving the key, instead notes that "if the password is the empty string, then
so is P" -- a zero-length block. Read strictly, B.1 is the formatting step and B.2 works on its output, so B.1
wins; implementations disagree anyway. The two produce different keys, so a file written under one reading fails
MAC verification under the other.

`openssl pkcs12 -export -passout pass:` writes the two-byte form, as do keytool and python-cryptography.
OpenSSL, Schannel, Java KeyStore and .NET read both forms. The two Apple platforms each read only one, and not
the same one: macOS takes only the zero-length form (SecItemImport reports errSecPassphraseRequired (-25260) or
errSecPkcs12VerifyFailure (-25264), SecPKCS12Import errSecAuthFailed (-25293), for the two-byte form), while
iOS takes only the two-byte form and rejects the zero-length one with errSecAuthFailed (-25293).

This script writes the zero-length form, so its output is the fixture for macOS; the openssl-generated
`*_password_less.p12` files remain the fixture for iOS.

The private key must go in a pkcs8ShroudedKeyBag: with a plain, unencrypted keyBag, SecPKCS12Import returns
the certificate but no identity, and Java's KeyStore reports no aliases at all.

PBE-SHA1-3DES and a SHA-1 MAC are used because Apple does not accept the PBES2/AES defaults, the same reason
makecerts.sh passes -legacy to openssl.
"""

import argparse
import hashlib
import hmac
import os

from cryptography import x509
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.ciphers import Cipher, modes

try:
    # cryptography 43 moved TripleDES here. Debian 12, used by the Dockerfile in this directory, ships 38.
    from cryptography.hazmat.decrepit.ciphers.algorithms import TripleDES
except ImportError:
    from cryptography.hazmat.primitives.ciphers.algorithms import TripleDES

# --- DER encoding -----------------------------------------------------------------------------------------


def _length(n: int) -> bytes:
    if n < 0x80:
        return bytes([n])
    encoded = n.to_bytes((n.bit_length() + 7) // 8, "big")
    return bytes([0x80 | len(encoded)]) + encoded


def tlv(tag: int, body: bytes) -> bytes:
    return bytes([tag]) + _length(len(body)) + body


def seq(*parts: bytes) -> bytes:
    return tlv(0x30, b"".join(parts))


def octets(body: bytes) -> bytes:
    return tlv(0x04, body)


def integer(value: int) -> bytes:
    return tlv(0x02, value.to_bytes(max(1, (value.bit_length() + 8) // 8), "big"))


def null() -> bytes:
    return b"\x05\x00"


def explicit(index: int, body: bytes) -> bytes:
    """[index] EXPLICIT body"""
    return tlv(0xA0 | index, body)


def oid(dotted: str) -> bytes:
    parts = [int(part) for part in dotted.split(".")]
    body = bytes([parts[0] * 40 + parts[1]])
    for part in parts[2:]:
        chunk = [part & 0x7F]
        part >>= 7
        while part:
            chunk.append(0x80 | (part & 0x7F))
            part >>= 7
        body += bytes(reversed(chunk))
    return tlv(0x06, body)


OID_DATA = oid("1.2.840.113549.1.7.1")
OID_ENCRYPTED_DATA = oid("1.2.840.113549.1.7.6")
OID_CERT_BAG = oid("1.2.840.113549.1.12.10.1.3")
OID_SHROUDED_KEY_BAG = oid("1.2.840.113549.1.12.10.1.2")
OID_X509_CERTIFICATE = oid("1.2.840.113549.1.9.22.1")
OID_PBE_SHA1_3DES = oid("1.2.840.113549.1.12.1.3")
OID_SHA1 = oid("1.3.14.3.2.26")
OID_LOCAL_KEY_ID = oid("1.2.840.113549.1.9.21")

# --- RFC 7292 appendix B.2 key derivation ---------------------------------------------------------------

# The empty password, encoded as a zero-length block rather than as the two 0x00 bytes of a NULL-terminated
# empty BMPString. This single value is what makes the output importable on macOS.
EMPTY_PASSWORD = b""

PURPOSE_KEY = 1
PURPOSE_IV = 2
PURPOSE_MAC = 3

_DIGEST_SIZE = 20  # SHA-1
_BLOCK_SIZE = 64  # SHA-1


def derive(salt: bytes, iterations: int, length: int, purpose: int) -> bytes:
    """RFC 7292 appendix B.2, specialized to SHA-1 and an empty password."""

    def fill(data: bytes) -> bytes:
        # v * ceil(len(data) / v) bytes of data repeated. An empty input stays empty, which is exactly how the
        # zero-length password differs from the two-byte one.
        if not data:
            return b""
        size = _BLOCK_SIZE * ((len(data) + _BLOCK_SIZE - 1) // _BLOCK_SIZE)
        return (data * (size // len(data) + 1))[:size]

    diversifier = bytes([purpose]) * _BLOCK_SIZE
    block = bytearray(fill(salt) + fill(EMPTY_PASSWORD))

    output = b""
    while len(output) < length:
        digest = diversifier + bytes(block)
        for _ in range(iterations):
            digest = hashlib.sha1(digest).digest()
        output += digest
        if len(output) >= length:
            break
        # Treat the digest as a big-endian number, add one, and add it to each block of I.
        repeated = (digest * ((_BLOCK_SIZE + _DIGEST_SIZE - 1) // _DIGEST_SIZE))[:_BLOCK_SIZE]
        addend = int.from_bytes(repeated, "big") + 1
        for offset in range(0, len(block), _BLOCK_SIZE):
            chunk = int.from_bytes(block[offset : offset + _BLOCK_SIZE], "big") + addend
            block[offset : offset + _BLOCK_SIZE] = (chunk % (1 << (_BLOCK_SIZE * 8))).to_bytes(_BLOCK_SIZE, "big")
    return output[:length]


def encrypt(plaintext: bytes, salt: bytes, iterations: int) -> bytes:
    """pbeWithSHA1And3-KeyTripleDES-CBC"""
    key = derive(salt, iterations, 24, PURPOSE_KEY)
    iv = derive(salt, iterations, 8, PURPOSE_IV)
    encryptor = Cipher(TripleDES(key), modes.CBC(iv)).encryptor()
    padding = 8 - (len(plaintext) % 8)
    return encryptor.update(plaintext + bytes([padding]) * padding) + encryptor.finalize()


def pbe_algorithm(salt: bytes, iterations: int) -> bytes:
    return seq(OID_PBE_SHA1_3DES, seq(octets(salt), integer(iterations)))


def local_key_id(value: bytes) -> bytes:
    """A localKeyId attribute, so that readers can pair the certificate with its private key.

    Without it the file still loads, but `openssl pkcs12 -clcerts` and PKCS12_parse cannot tell which
    certificate the key belongs to.
    """
    return tlv(0x31, seq(OID_LOCAL_KEY_ID, tlv(0x31, octets(value))))


# --- PKCS#12 ----------------------------------------------------------------------------------------------


def build(key_pem: bytes, certificate_pem: bytes, iterations: int = 2048) -> bytes:
    key = serialization.load_pem_private_key(key_pem, password=None)
    certificate = x509.load_pem_x509_certificate(certificate_pem)

    certificate_der = certificate.public_bytes(serialization.Encoding.DER)
    key_der = key.private_bytes(
        serialization.Encoding.DER,
        serialization.PrivateFormat.PKCS8,
        serialization.NoEncryption(),
    )
    key_id = hashlib.sha1(certificate_der).digest()

    # The certificate, in an encryptedData ContentInfo.
    certificate_bag = seq(
        OID_CERT_BAG,
        explicit(0, seq(OID_X509_CERTIFICATE, explicit(0, octets(certificate_der)))),
        local_key_id(key_id),
    )
    certificate_salt = os.urandom(8)
    certificate_content = seq(
        OID_ENCRYPTED_DATA,
        explicit(
            0,
            seq(
                integer(0),
                seq(
                    OID_DATA,
                    pbe_algorithm(certificate_salt, iterations),
                    # encryptedContent is [0] IMPLICIT OCTET STRING.
                    tlv(0x80, encrypt(seq(certificate_bag), certificate_salt, iterations)),
                ),
            ),
        ),
    )

    # The private key, as a pkcs8ShroudedKeyBag in a plain data ContentInfo.
    key_salt = os.urandom(8)
    key_bag = seq(
        OID_SHROUDED_KEY_BAG,
        explicit(0, seq(pbe_algorithm(key_salt, iterations), octets(encrypt(key_der, key_salt, iterations)))),
        local_key_id(key_id),
    )
    key_content = seq(OID_DATA, explicit(0, octets(seq(key_bag))))

    authenticated_safe = seq(certificate_content, key_content)

    mac_salt = os.urandom(20)
    mac = hmac.new(derive(mac_salt, iterations, 20, PURPOSE_MAC), authenticated_safe, hashlib.sha1).digest()
    mac_data = seq(seq(seq(OID_SHA1, null()), octets(mac)), octets(mac_salt), integer(iterations))

    return seq(integer(3), seq(OID_DATA, explicit(0, octets(authenticated_safe))), mac_data)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--inkey", required=True, help="PEM private key file")
    parser.add_argument("--in", dest="certificate", required=True, help="PEM certificate file")
    parser.add_argument("--out", required=True, help="PKCS#12 file to write")
    args = parser.parse_args()

    with open(args.inkey, "rb") as file:
        key_pem = file.read()
    with open(args.certificate, "rb") as file:
        certificate_pem = file.read()

    with open(args.out, "wb") as file:
        file.write(build(key_pem, certificate_pem))


if __name__ == "__main__":
    main()
