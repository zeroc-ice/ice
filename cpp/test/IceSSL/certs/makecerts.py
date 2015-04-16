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

if not IceCertUtils.CertificateUtils.opensslSupport:
    print("openssl is required to generate the test certificates")
    sys.exit(1)

def usage():
    print("Usage: " + sys.argv[0] + " [options]")
    print("")
    print("Options:")
    print("-h               Show this message.")
    print("-d | --debug     Debugging output.")
    print("--clean          Clean the current database first.")
    sys.exit(1)

#
# Check arguments
#
debug = False
clean = False
try:
    opts, args = getopt.getopt(sys.argv[1:], "hd", ["help", "debug", "clean"])
except getopt.GetoptError as e:
    print("Error %s " % e)
    usage()
    sys.exit(1)

for (o, a) in opts:
    if o == "-h" or o == "--help":
        usage()
        sys.exit(0)
    elif o == "-d" or o == "--debug":
        debug = True
    elif o == "--clean":
        clean = True

home = os.path.join(os.path.dirname(os.path.abspath(__file__)), "db")
homeca1 = os.path.join(home, "ca1")
homeca2 = os.path.join(home, "ca2")
if not os.path.exists("db"):
    os.mkdir(home)
    os.mkdir(homeca1)
    os.mkdir(homeca2)

if clean:
    for h in [homeca1, homeca2]:
        IceCertUtils.CertificateFactory(home=h).destroy(True)

# Create 2 CAs, the DSA ca is actually ca1 but with a different the DSA key generation algorithm.
ca1 = IceCertUtils.CertificateFactory(home=homeca1, cn="ZeroC Test CA 1", ip="127.0.0.1", email="issuer@zeroc.com")
ca2 = IceCertUtils.CertificateFactory(home=homeca2, cn="ZeroC Test CA 2", ip="127.0.0.1", email="issuer@zeroc.com")
dsaca = IceCertUtils.OpenSSLCertificateFactory(home=ca1.home, keyalg="dsa", keysize=1024)

#
# Export CA certificates
#
ca1.getCA().save("cacert1.pem")
ca2.getCA().save("cacert2.pem")

# Also export the ca2 self-signed certificate, it's used by the tests to test self-signed certificates
ca2.getCA().saveKey("cakey2.pem").save("cacert2.p12", addkey=True)

#
# Generate certificates (CA, alias, { creation parameters passed to ca.create(...) }, password)
#
certs = [
    (ca1, "s_rsa_ca1",      { "cn": "Server", "ip": "127.0.0.1", "dns": "server", "serial": 1 }, None),
    (ca1, "c_rsa_ca1",      { "cn": "Client", "ip": "127.0.0.1", "dns": "client", "serial": 2 }, None),
    (ca1, "s_rsa_pass_ca1", { "cn": "Server", "ip": "127.0.0.1", "dns": "server", "serial": 1 }, "server"),
    (ca1, "c_rsa_pass_ca1", { "cn": "Client", "ip": "127.0.0.1", "dns": "client", "serial": 2 }, "client"),
    (ca1, "s_rsa_ca1_exp",  { "cn": "Server", "validity": -1 }, None), # Expired certificate
    (ca1, "c_rsa_ca1_exp",  { "cn": "Client", "validity": -1 }, None), # Expired certificate
    (ca1, "s_rsa_ca1_cn1",  { "cn": "127.0.0.1" }, None),  # No subjectAltName, CN=127.0.0.1
    (ca1, "s_rsa_ca1_cn2",  { "cn": "127.0.0.11" }, None), # No subjectAltName, CN=127.0.0.11
    (ca2, "s_rsa_ca2",      { "cn": "Server", "ip": "127.0.0.1", "dns": "server" }, None),
    (ca2, "c_rsa_ca2",      { "cn": "Client", "ip": "127.0.0.1", "dns": "client" }, None),
    (dsaca, "s_dsa_ca1",    { "cn": "Server", "ip": "127.0.0.1", "dns": "server" }, None), # DSA
    (dsaca, "c_dsa_ca1",    { "cn": "Client", "ip": "127.0.0.1", "dns": "client" }, None), # DSA
]

#
# Save the certificates in PEM and PKCS12 format.
#
for (ca, alias, args, password) in certs:
    #
    # Get or create the certificate
    #
    cert = ca.get(alias) or ca.create(alias, **args)

    #
    # Save it as PEM and PKCS12
    #
    cert.save(alias + "_pub.pem").saveKey(alias + "_priv.pem", password).save(alias + ".p12", password)

#
# Create DH parameters to use with OS X Secure Transport.
#
if clean or not os.path.exists("dh_params512.der"):
    ca1.run("openssl", "dhparam", 512, outform="DER", out="dh_params512.der")
if clean or not os.path.exists("dh_params1024.der"):
    ca1.run("openssl", "dhparam", 1024, outform="DER", out="dh_params1024.der")
