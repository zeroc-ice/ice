This directory contains certificates that are required by the tests in
test/IceSSL/configuration. The 'makecerts.py' script generates
certificates in the current directory using the CA databases stored in
the cpp/test/IceSSL/certs/db directory.

Running this script, we'll just re-save the certificates in the
current directory without creating new certificates.

See cpp/test/IceSSL/certs/README.md for more information
