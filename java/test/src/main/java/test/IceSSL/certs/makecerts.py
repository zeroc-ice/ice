#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2015 ZeroC, Inc. All rights reserved.
#
# **********************************************************************

import os, sys, socket, getopt

try:
    import IceCertUtils
except:
    print("error: couldn't find IceCertUtils, install `zeroc-ice-certutils' package "
          "from Python package repository")
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

ca1.getCA().save("cacert1.jks")
ca2.getCA().save("cacert2.jks")

certs = [
    (ca1, "s_rsa_ca1"),
    (ca1, "c_rsa_ca1"),
    (ca1, "s_rsa_ca1_exp"), # Expired certificate
    (ca1, "c_rsa_ca1_exp"), # Expired certificate
    (ca1, "s_rsa_ca1_cn1"), # No subjectAltName, CN=127.0.0.1
    (ca1, "s_rsa_ca1_cn2"), # No subjectAltName, CN=127.0.0.11
    (ca2, "s_rsa_ca2"),
    (ca2, "c_rsa_ca2"),
    (ca1, "s_dsa_ca1"),
    (ca1, "c_dsa_ca1"),
]

#
# Save the certificate Java certificate to JKS files that contain both
# the certificate and the CA certificate. We also ensure the certificate
# chain contains the root certficiate.
#
for (ca, alias) in certs:
    cert = ca.get(alias) or ca.create(alias, **args)
    cert.save(alias + ".jks", alias="cert", caalias="cacert", root=True)

#
# Create a cacerts.jks truststore that contains both CA certs.
#
if os.path.exists("cacerts.jks"): os.remove("cacerts.jks")
ca1.getCA().exportToKeyStore("cacerts.jks", alias="cacert1")
ca2.getCA().exportToKeyStore("cacerts.jks", alias="cacert2")

ca2.getCA().saveJKS("s_cacert2.jks", addkey=True)

#
# Create a client/server certificate that contains both the DSA and
# RSA certs.
#
if os.path.exists("s_rsa_dsa_ca1.jks"): os.remove("s_rsa_dsa_ca1.jks")
ca1.get("s_rsa_ca1").exportToKeyStore("s_rsa_dsa_ca1.jks", alias="rsacert")
ca1.get("s_dsa_ca1").exportToKeyStore("s_rsa_dsa_ca1.jks", alias="dsacert")
