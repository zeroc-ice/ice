This directory contains certificates that are required by the tests in
test/IceSSL/configuration. The shell script 'makecerts' generates a
new set of certificates. Note that these certificates are also used by
other language mappings, therefore if you generate a new set here, you
should also run the makecerts scripts in the other language mappings
so that all of the certificates are consistent.

Also note that there are some backward-compatibility issues between
OpenSSL 1.0 and OpenSSL 0.9.x. One issue that affects the certificates
in this directory is the hashing scheme that OpenSSL uses to index
certificates and locate them at run time. This scheme changed in
OpenSSL 1.0, consequently a certificate repository that was indexed
using the c_rehash script from OpenSSL 0.9.x will not work properly in
OpenSSL 1.0 until the directory is reindexed using the new version of
c_rehash.

The makecerts script does not use c_rehash because it creates symbolic
links to the certificates, and symbolic links are not very portable.
Instead, makecerts creates copies of the certificate files using the
same naming scheme as c_rehash. makecerts actually generates two
copies of the certificates; one set uses the naming scheme for OpenSSL
1.0, and the other uses the naming scheme for OpenSSL 0.9.x.

makecerts requires that you use OpenSSL 1.0 to run the script.
