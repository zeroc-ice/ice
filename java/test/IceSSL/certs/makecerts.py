#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil

for toplevel in [".", "..", "../..", "../../..", "../../../..", "../../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "scripts", "TestUtil.py")):
        break
else:
    raise RuntimeError("can't find toplevel directory!")

sys.path.append(toplevel)
from scripts import *

#
# Show usage information.
#
def usage():
    print "Usage: " + sys.argv[0] + " [options]"
    print
    print "Options:"
    print "-h    Show this message."
    print "-f    Force an update to the Java files."

#
# Check arguments
#
force = 0
for x in sys.argv[1:]:
    if x == "-h":
        usage()
        sys.exit(0)
    elif x == "-f":
        force = 1
    elif x.startswith("-"):
        print sys.argv[0] + ": unknown option `" + x + "'"
        print
        usage()
        sys.exit(1)
    else:
        usage()
        sys.exit(1)

cppcerts = os.path.join(TestUtil.getIceDir("cpp"), "test", "IceSSL", "certs")

certs = [\
    "c_dsa_nopass_ca1", \
    "c_rsa_nopass_ca1_exp", \
    "c_rsa_nopass_ca1", \
    "c_rsa_nopass_ca2", \
    "s_dsa_nopass_ca1", \
    "s_rsa_nopass_ca1_exp", \
    "s_rsa_nopass_ca1", \
    "s_rsa_nopass_ca2", \
    "s_rsa_nopass_ca1_cn1", \
    "s_rsa_nopass_ca1_cn2", \
]

#
# Create truststores from the CA certificates.
#
for x in ("cacert1", "cacert2"):
    ts = x + ".jks"
    os.system("openssl x509 -in " + os.path.join(cppcerts, x) + ".pem -outform DER -out " + x + ".der")
    if force or not os.path.exists(ts):
        if os.path.exists(ts):
            os.remove(ts)
        os.system("keytool -import -alias cacert -file " + x + ".der -keystore " + ts + \
                  " -storepass password -noprompt")
        print "Created " + ts

#
# Create a truststore containing both CA certificates.
#
if force or not os.path.exists("cacerts.jks"):
    if os.path.exists("cacerts.jks"):
        os.remove("cacerts.jks")
    os.system("keytool -import -alias cacert1 -file cacert1.der -keystore cacerts.jks -storepass password -noprompt")
    os.system("keytool -import -alias cacert2 -file cacert2.der -keystore cacerts.jks -storepass password -noprompt")
    print "Created cacerts.jks"

#
# Convert key/certificate pairs into PKCS12 format and then import them
# into keystores.
#
for x in certs:
    p12 = x.replace("nopass_", "") + ".p12"
    ks = x.replace("nopass_", "") + ".jks"
    if x.find("1") > 0:
        cacert = "cacert1"
    else:
        cacert = "cacert2"
    if force or not os.path.exists(ks):
        if os.path.exists(ks):
            os.remove(ks)
        cert = os.path.join(cppcerts, x)
        ca = os.path.join(cppcerts, cacert) + ".pem"
        os.system("openssl pkcs12 -in " + cert + "_pub.pem -inkey " + cert + "_priv.pem -export -out " + p12 + \
                  " -name cert -passout pass:password -certfile " + ca)
        os.system("java -classpath ../../../../certs ImportKey " + p12  + " cert " + cacert + ".der " + ks + " password")
        os.remove(p12)
        print "Created " + ks

#
# Create a keystore that contains both RSA and DSS certificates.
#
ks = "s_rsa_dsa_ca1.jks"
if force or not os.path.exists(ks):
    if os.path.exists(ks):
        os.remove(ks)
    cacert = "cacert1"
    ca = os.path.join(cppcerts, cacert) + ".pem"
    p12 = "s_dsa_nopass_ca1.p12"
    cert = os.path.join(cppcerts, "s_dsa_nopass_ca1")
    os.system("openssl pkcs12 -in " + cert + "_pub.pem -inkey " + cert + "_priv.pem -export -out " + p12 + \
              " -name dsacert -passout pass:password -certfile " + ca)
    os.system("java -classpath ../../../../certs ImportKey " + p12  + " dsacert " + cacert + ".der " + ks + " password")
    os.remove(p12)
    p12 = "s_rsa_nopass_ca1.p12"
    cert = os.path.join(cppcerts, "s_rsa_nopass_ca1")
    os.system("openssl pkcs12 -in " + cert + "_pub.pem -inkey " + cert + "_priv.pem -export -out " + p12 + \
              " -name rsacert -passout pass:password -certfile " + ca)
    os.system("java -classpath ../../../../certs ImportKey " + p12  + " rsacert " + cacert + ".der " + ks + " password")
    os.remove(p12)
    print "Created " + ks

#
# Clean up.
#
for x in ("cacert1", "cacert2"):
    cert = x + ".der"
    if os.path.exists(cert):
        os.remove(cert)
#
# Done.
#
print "Done."
