This directory contains certificates that are required by the tests in
test/IceSSL/configuration. The 'makecerts.py' script generates
certificates in the current directory using the CA databases stored
under the db directory.

You can use iceca from the zeroc-icecertutils PyPi package to view
the content of the CA databases, for example:

    ICE_CA_HOME=db/ca1 iceca --capass=password list

Running this script will just re-save the certificates in the current
directory without creating new certificates.

If you want to create a new set of certificates, you should run the
scripts with --clean to clean the CA databases.

WARNING: re-generating the certificates will require fixing the tests
to account for the new subject key IDs, etc.
