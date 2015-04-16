#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2015 ZeroC, Inc. All rights reserved.
#
# **********************************************************************

import os, sys, socket, getopt

try:
    import IceCertUtils
except Exception as ex:
    print("error: couldn't find IceCertUtils, install `zeroc-ice-certutils' package "
          "from Python package repository:\n" + str(ex))
    sys.exit(1)

toplevel="."
while(toplevel != "/"):
    toplevel = os.path.normpath(os.path.join("..", toplevel))
    if os.path.exists(os.path.join(toplevel, "scripts", "TestUtil.py")):
        break
else:
    raise RuntimeError("can't find toplevel directory!")

cppcerts = os.path.join(toplevel, "cpp", "test", "IceSSL", "certs")
if not os.path.exists(os.path.join(cppcerts, "db", "ca1", "ca.pem")):
    print("error: CA database is not initialized in `" + os.path.join(cppcerts, "db") + "',"
          " run makecerts.py in `" + cppcerts + "' first")
    sys.exit(1)

ca1 = IceCertUtils.CertificateFactory(home=os.path.join(cppcerts, "db", "ca1"))
ca2 = IceCertUtils.CertificateFactory(home=os.path.join(cppcerts, "db", "ca2"))

ca1.getCA().save("cacert1.pem")
ca2.getCA().save("cacert2.pem")

certs = [
    (ca1, "s_rsa_ca1"),
    (ca1, "c_rsa_ca1"),
    (ca1, "s_rsa_ca1_exp"), # Expired certificate
    (ca1, "c_rsa_ca1_exp"), # Expired certificate
    (ca1, "s_rsa_ca1_cn1"), # No subjectAltName, CN=127.0.0.1
    (ca1, "s_rsa_ca1_cn2"), # No subjectAltName, CN=127.0.0.11
    (ca2, "s_rsa_ca2"),
    (ca2, "c_rsa_ca2"),
]

#
# Save the certificate as PKCS12 files.
#
for (ca, alias) in certs:
    cert = ca.get(alias) or ca.create(alias, **args)
    cert.save(alias + ".p12")

# Also export the ca2 self-signed certificate, it's used by the tests to test self-signed certificates
ca2.getCA().save("cacert2.p12", addkey=True)
