#!/usr/bin/env python3
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import os, sys, socket, getopt

try:
    import IceCertUtils
except:
    print("error: couldn't find IceCertUtils, install `zeroc-icecertutils' package "
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
    print("--clean          Clean the CA database first.")
    print("--force          Re-save all the files even if they already exists.")
    sys.exit(1)

def runCommands(commands):
    for command in commands:
        if os.system(command) != 0:
            print("error running command `{0}'".format(command))
            sys.exit(1)

#
# Check arguments
#
debug = False
clean = False
force = False
try:
    opts, args = getopt.getopt(sys.argv[1:], "hd", ["help", "debug", "clean", "force"])
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
    elif o == "--force":
        force = True

home = os.path.join(os.path.dirname(os.path.abspath(__file__)), "db")
homeca1 = os.path.join(home, "ca1")
homeca2 = os.path.join(home, "ca2")
homeca3 = os.path.join(home, "ca3")
homeca4 = os.path.join(home, "ca4")
if not os.path.exists("db"):
    os.mkdir(home)
    os.mkdir(homeca1)
    os.mkdir(homeca2)
    os.mkdir(homeca3)
    os.mkdir(homeca4)

if clean:
    for h in [homeca1, homeca2, homeca3]:
        IceCertUtils.CertificateFactory(home=h).destroy(True)

#
# Create 2 CAs, the DSA ca is actually ca1 but with a different the DSA key generation algorithm.
# ca2 is also used as a server. The serverAuth extension is required on some OSs (macOS Catalina)
#
ca1 = IceCertUtils.CertificateFactory(home=homeca1, cn="ZeroC Test CA 1", ip="127.0.0.1", email="issuer@zeroc.com")
ca2 = IceCertUtils.CertificateFactory(home=homeca2, cn="ZeroC Test CA 2", ip="127.0.0.1", email="issuer@zeroc.com",
                                      extendedKeyUsage="serverAuth")
# CA3 include CRL distribution points
ca3 = IceCertUtils.CertificateFactory(
    home=homeca3,
    cn="ZeroC Test CA 3",
    ip="127.0.0.1",
    email="issuer@zeroc.com",
    crlDistributionPoints="http://127.0.0.1:20001/ca3.crl.pem")

# CA4 include AIA extension
ca4 = IceCertUtils.CertificateFactory(
    home=homeca4,
    cn="ZeroC Test CA 4",
    ip="127.0.0.1",
    email="issuer@zeroc.com",
    ocspResponder="http://127.0.0.1:20002",
    caIssuers="http://127.0.0.1:20001/cacert4.der")

dsaca = IceCertUtils.OpenSSLCertificateFactory(home=ca1.home, keyalg="dsa", keysize=2048)

#
# Export CA certificates
#
for ca, name in [(ca1, "cacert1"), (ca2, "cacert2"), (ca3, "cacert3"), (ca4, "cacert4")]:
    pem = "{}.pem".format(name)
    if force or not os.path.exists(pem): ca.getCA().save(pem)
    der = "{}.der".format(name)
    if force or not os.path.exists(der): ca.getCA().save(der)

if force or not os.path.exists("cacerts.pem"):
    pem = ""
    with open("cacert1.pem", "r") as f: pem += f.read()
    with open("cacert2.pem", "r") as f: pem += f.read()
    with open("cacert3.pem", "r") as f: pem += f.read()
    with open("cacert4.pem", "r") as f: pem += f.read()
    with open("cacerts.pem", "w") as f: f.write(pem)

# Also export the ca2 self-signed certificate, it's used by the tests to test self-signed certificates
if force or not os.path.exists("cacert2_pub.pem"): ca2.getCA().save("cacert2_pub.pem")
if force or not os.path.exists("cacert2_priv.pem"): ca2.getCA().saveKey("cacert2_priv.pem")
if force or not os.path.exists("cacert2.p12"): ca2.getCA().save("cacert2.p12", addkey=True)

# Create intermediate CAs
cai1 = ca1.getIntermediateFactory("intermediate1")
if not cai1:
    cai1 = ca1.createIntermediateFactory("intermediate1", cn = "ZeroC Test Intermediate CA 1")

cai2 = cai1.getIntermediateFactory("intermediate1")
if not cai2:
    cai2 = cai1.createIntermediateFactory("intermediate1", cn = "ZeroC Test Intermediate CA 2")

cai3 = ca3.getIntermediateFactory("intermediate1")
if not cai3:
    cai3 = ca3.createIntermediateFactory("intermediate1",
                                         cn="ZeroC Test Intermediate CA 3",
                                         crlDistributionPoints="http://127.0.0.1:20001/cai3.crl.pem")

if force or not os.path.exists("cai3.pem"): cai3.getCA().save("cai3.pem")
if force or not os.path.exists("cai3.der"): cai3.getCA().save("cai3.der")

cai4 = ca4.getIntermediateFactory("intermediate1")
if not cai4:
    cai4 = ca4.createIntermediateFactory("intermediate1",
                                         cn="ZeroC Test Intermediate CA 4",
                                         ocspResponder="http://127.0.0.1:20002",
                                         caIssuers="http://127.0.0.1:20001/cai4.der")

if force or not os.path.exists("cai4.pem"): cai4.getCA().save("cai4.pem")
if force or not os.path.exists("cai4.der"): cai4.getCA().save("cai4.der")

#
# Create certificates (CA, alias, { creation parameters passed to ca.create(...) })
#
certs = [
    (ca1, "s_rsa_ca1",       { "cn": "Server", "ip": "127.0.0.1", "dns": "server", "serial": 1 }),
    (ca1, "c_rsa_ca1",       { "cn": "Client", "ip": "127.0.0.1", "dns": "client", "serial": 2 }),
    (ca1, "s_rsa_ca1_exp",   { "cn": "Server", "validity": -1 }), # Expired certificate
    (ca1, "c_rsa_ca1_exp",   { "cn": "Client", "validity": -1 }), # Expired certificate

    (ca1, "s_rsa_ca1_cn1",   { "cn": "Server", "dns": "localhost" }),       # DNS subjectAltName localhost
    (ca1, "s_rsa_ca1_cn2",   { "cn": "Server", "dns": "localhostXX" }),     # DNS subjectAltName localhostXX
    (ca1, "s_rsa_ca1_cn3",   { "cn": "localhost" }),                        # No subjectAltName, CN=localhost
    (ca1, "s_rsa_ca1_cn4",   { "cn": "localhostXX" }),                      # No subjectAltName, CN=localhostXX
    (ca1, "s_rsa_ca1_cn5",   { "cn": "localhost", "dns": "localhostXX" }),  # DNS subjectAltName localhostXX, CN=localhost
    (ca1, "s_rsa_ca1_cn6",   { "cn": "Server", "ip": "127.0.0.1" }),        # IP subjectAltName 127.0.0.1
    (ca1, "s_rsa_ca1_cn7",   { "cn": "Server", "ip": "127.0.0.2" }),        # IP subjectAltName 127.0.0.2
    (ca1, "s_rsa_ca1_cn8",   { "cn": "127.0.0.1" }),                        # No subjectAltName, CN=127.0.0.1
    (ca1, "s_rsa_ca1_cn9",   { "cn": "127.0.0.1", "ip": ["127.0.0.1", "::1"] }),
    (ca1, "s_rsa_ca1_cn10",   { "cn": "127.0.0.1", "dns": ["host1", "host2"] }),
    (ca1, "s_rsa_ca1_cn11",   { "cn": "127.0.0.1", "ip": ["127.0.0.1", "127.0.0.2"], "dns": ["host1", "host2"] }),

    (ca2, "s_rsa_ca2",       { "cn": "Server", "ip": "127.0.0.1", "dns": "server" }),
    (ca2, "c_rsa_ca2",       { "cn": "Client", "ip": "127.0.0.1", "dns": "client" }),
    (dsaca, "s_dsa_ca1",     { "cn": "Server", "ip": "127.0.0.1", "dns": "server" }), # DSA
    (dsaca, "c_dsa_ca1",     { "cn": "Client", "ip": "127.0.0.1", "dns": "client" }), # DSA
    (cai1, "s_rsa_cai1",     { "cn": "Server", "ip": "127.0.0.1", "dns": "server" }),
    (cai2, "s_rsa_cai2",     { "cn": "Server", "ip": "127.0.0.1", "dns": "server" }),
    (cai2, "c_rsa_cai2",     { "cn": "Client", "ip": "127.0.0.1", "dns": "client" }),

    # Certificates with CRL distribution points
    (ca3, "s_rsa_ca3",           { "cn": "Server", "ip": "127.0.0.1", "dns": "server" }),
    (ca3, "c_rsa_ca3",           { "cn": "Client", "ip": "127.0.0.1", "dns": "client" }),
    (ca3, "s_rsa_ca3_revoked",   { "cn": "Server ca3 revoked", "ip": "127.0.0.1", "dns": "server" }),
    (ca3, "c_rsa_ca3_revoked",   { "cn": "Client ca3 revoked", "ip": "127.0.0.1", "dns": "client" }),

    (cai3, "s_rsa_cai3",         { "cn": "Server cai3", "ip": "127.0.0.1", "dns": "server" }),
    (cai3, "c_rsa_cai3",         { "cn": "Client cai3", "ip": "127.0.0.1", "dns": "client" }),
    (cai3, "s_rsa_cai3_revoked", { "cn": "Server cai3 revoked", "ip": "127.0.0.1", "dns": "server" }),
    (cai3, "c_rsa_cai3_revoked", { "cn": "Client cai3 revoked", "ip": "127.0.0.1", "dns": "client" }),

    # Certificates with AIA extension
    (ca4, "s_rsa_ca4",           { "cn": "Server ca4", "ip": "127.0.0.1", "dns": "server" }),
    (ca4, "c_rsa_ca4",           { "cn": "Client ca4", "ip": "127.0.0.1", "dns": "client" }),
    (ca4, "s_rsa_ca4_revoked",   { "cn": "Server ca4 revoked", "ip": "127.0.0.1", "dns": "server" }),
    (ca4, "c_rsa_ca4_revoked",   { "cn": "Client ca4 revoked", "ip": "127.0.0.1", "dns": "client" }),
    # The OCSP responder doesn't know about this certs
    (ca4, "s_rsa_ca4_unknown", { "cn": "Server ca4 unknown", "ip": "127.0.0.1", "dns": "server" }),
    (ca4, "c_rsa_ca4_unknown", { "cn": "Client ca4 unknown", "ip": "127.0.0.1", "dns": "client" }),

    (cai4, "s_rsa_cai4",         { "cn": "Server cai4", "ip": "127.0.0.1", "dns": "server" }),
    (cai4, "c_rsa_cai4",         { "cn": "Client cai4", "ip": "127.0.0.1", "dns": "client" }),
    (cai4, "s_rsa_cai4_revoked", { "cn": "Server cai4 revoked", "ip": "127.0.0.1", "dns": "server" }),
    (cai4, "c_rsa_cai4_revoked", { "cn": "Client cai4 revoked", "ip": "127.0.0.1", "dns": "client" }),
]

#
# Create the certificates
#
for (ca, alias, args) in certs:
    if not ca.get(alias):
        ca.create(alias, extendedKeyUsage="clientAuth" if alias.startswith("c_") else "serverAuth", **args)

# Additional certs for extended key usage testing
certs = [
    (ca1, "rsa_ca1_none", None, { "cn": "None"}),
    (ca1, "rsa_ca1_serverAuth", "serverAuth", { "cn": "Server Auth"}),
    (ca1, "rsa_ca1_clientAuth", "clientAuth", { "cn": "Client Auth"}),
    (ca1, "rsa_ca1_codeSigning", "codeSigning", { "cn": "Code Signing"}),
    (ca1, "rsa_ca1_emailProtection", "emailProtection", { "cn": "Email Protection"}),
    (ca1, "rsa_ca1_timeStamping", "timeStamping", { "cn": "Time Stamping"}),
    (ca1, "rsa_ca1_ocspSigning", "OCSPSigning", {"cn": "OCSP Signing"}),
    (ca1, "rsa_ca1_anyExtendedKeyUsage", "anyExtendedKeyUsage", {"cn": "Any Extended Key Usage"})
]

for (ca, alias, extendedKeyUsage, args) in certs:
    if not ca.get(alias):
        ca.create(alias, extendedKeyUsage=extendedKeyUsage, **args)

savecerts = [
    (ca1, "s_rsa_ca1",     None,              {}),
    (ca1, "c_rsa_ca1",     None,              {}),
    (ca1, "s_rsa_ca1_exp", None,              {}),
    (ca1, "c_rsa_ca1_exp", None,              {}),
    (ca1, "s_rsa_ca1_cn1", None,              {}),
    (ca1, "s_rsa_ca1_cn2", None,              {}),
    (ca1, "s_rsa_ca1_cn3", None,              {}),
    (ca1, "s_rsa_ca1_cn4", None,              {}),
    (ca1, "s_rsa_ca1_cn5", None,              {}),
    (ca1, "s_rsa_ca1_cn6", None,              {}),
    (ca1, "s_rsa_ca1_cn7", None,              {}),
    (ca1, "s_rsa_ca1_cn8", None,              {}),
    (ca1, "s_rsa_ca1_cn9", None,              {}),
    (ca1, "s_rsa_ca1_cn10", None,             {}),
    (ca1, "s_rsa_ca1_cn11", None,             {}),

    (ca1, "rsa_ca1_none", None,                {}),
    (ca1, "rsa_ca1_serverAuth", None,          {}),
    (ca1, "rsa_ca1_clientAuth", None,          {}),
    (ca1, "rsa_ca1_codeSigning", None,         {}),
    (ca1, "rsa_ca1_emailProtection", None,     {}),
    (ca1, "rsa_ca1_timeStamping", None,        {}),
    (ca1, "rsa_ca1_ocspSigning", None,         {}),
    (ca1, "rsa_ca1_anyExtendedKeyUsage", None, {}),

    (ca2, "s_rsa_ca2",     None,              {}),
    (ca2, "c_rsa_ca2",     None,              {}),
    (dsaca, "s_dsa_ca1",   None,              {}),
    (dsaca, "c_dsa_ca1",   None,              {}),
    (cai1, "s_rsa_cai1",   None,              {}),
    (cai2, "s_rsa_cai2",   None,              {}),
    (cai2, "c_rsa_cai2",   None,              {}),
    (ca1, "s_rsa_ca1",     "s_rsa_wroot_ca1", { "root": True }),
    (ca1, "s_rsa_ca1",     "s_rsa_pass_ca1",  { "password": "server" }),
    (ca1, "c_rsa_ca1",     "c_rsa_pass_ca1",  { "password": "client" }),

    (ca3, "s_rsa_ca3",     None, {} ),
    (ca3, "c_rsa_ca3",     None, {} ),
    (ca3, "s_rsa_ca3_revoked",     None, {} ),
    (ca3, "c_rsa_ca3_revoked",     None, {} ),

    (cai3, "s_rsa_cai3",           None, {} ),
    (cai3, "c_rsa_cai3",           None, {} ),
    (cai3, "s_rsa_cai3_revoked",   None, {} ),
    (cai3, "c_rsa_cai3_revoked",   None, {} ),

    (ca4, "s_rsa_ca4",     None, {} ),
    (ca4, "c_rsa_ca4",     None, {} ),
    (ca4, "s_rsa_ca4_revoked",     None, {} ),
    (ca4, "c_rsa_ca4_revoked",     None, {} ),
    (ca4, "s_rsa_ca4_unknown",     None, {} ),
    (ca4, "c_rsa_ca4_unknown",     None, {} ),

    (cai4, "s_rsa_cai4",           None, {} ),
    (cai4, "c_rsa_cai4",           None, {} ),
    (cai4, "s_rsa_cai4_revoked",   None, {} ),
    (cai4, "c_rsa_cai4_revoked",   None, {} ),
]

#
# Save the certificates in PEM and PKCS12 format.
#
for (ca, alias, path, args) in savecerts:
    if not path: path = alias
    password = args.get("password", None)
    cert = ca.get(alias)
    if force or not os.path.exists(path + "_pub.pem"):
        cert.save(path + "_pub.pem")
    if force or not os.path.exists(path + "_priv.pem"):
        cert.saveKey(path + "_priv.pem", password)
    if force or not os.path.exists(path + ".p12"):
        cert.save(path + ".p12", **args)

#
# Create DH parameters to use with macOS Secure Transport.
#
for size in [512, 1024]:
    dhparams = "dh_params{0}.der".format(size)
    if clean or not os.path.exists(dhparams):
        ca1.run("openssl dhparam -outform DER -out {0} {1}".format(dhparams, size))

#
# Create certificate with custom extensions
#
if not os.path.exists("cacert_custom.pem"):
    commands = ["openssl req -new -key cakey1.pem -out cacert_custom.csr -config cacert_custom.req",
                "openssl x509 -req -in cacert_custom.csr -signkey cakey1.pem -out cacert_custom.pem -extfile cacert_custom.ext"]
    for command in commands:
        if os.system(command) != 0:
            print("error running command `{0}'".format(command))
            sys.exit(1)

    if os.path.exists("cacert_custom.csr"):
        os.remove("cacert_custom.csr")

def revokeCertificates(ca, cadir, certs):
    crlnumber = "{}/crlnumber".format(cadir)
    crlindex = "{}/index.txt".format(cadir)

    if os.path.exists(crlnumber):
        os.unlink(crlnumber)

    with open(crlnumber, "w") as f:
        f.write("00")

    if os.path.exists(crlindex):
        os.unlink(crlindex)

    with open(crlindex, "w") as f:
        f.write("")

    commands = []
    for cert in certs:
        commands.append("openssl ca -config {ca}.cnf -revoke {cadir}/{cert} -passin pass:password".format(
            ca=ca,
            cadir=cadir,
            cert=cert))

    commands.append(
        "openssl ca -config {ca}.cnf -gencrl -out {ca}.crl.pem -crldays 825 -passin pass:password".format(ca=ca))
    runCommands(commands)

crlfile = "ca.crl.pem"
if force or not os.path.exists(crlfile):
    revokeCertificates("ca3", "db/ca3", ["s_rsa_ca3_revoked.pem",
                                         "c_rsa_ca3_revoked.pem",
                                         "intermediate1/ca.pem"])
    revokeCertificates("cai3", "db/ca3/intermediate1", ["s_rsa_cai3_revoked.pem", "c_rsa_cai3_revoked.pem"])

    revokeCertificates("ca4", "db/ca4", ["s_rsa_ca4_revoked.pem",
                                         "c_rsa_ca4_revoked.pem",
                                         "intermediate1/ca.pem"])
    revokeCertificates("cai4", "db/ca4/intermediate1", ["s_rsa_cai4_revoked.pem", "c_rsa_cai4_revoked.pem"])


    # Concatenate CRL files
    if os.path.exists(crlfile):
        os.unlink(crlfile)

    with open(crlfile, "w") as outfile:
        for ca in ["ca3", "cai3"]:
            with open("{}.crl.pem".format(ca), "r") as infile:
                outfile.write(infile.read())
            outfile.write("\n")
