#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, atexit, re

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

# Filter-out the deprecated property warnings
TestUtil.clientTraceFilters = [ lambda x: re.sub("-! .* warning: deprecated property: IceSSL.KeyFile\n", "", x) ]
TestUtil.serverTraceFilters = [ lambda x: re.sub("-! .* warning: deprecated property: IceSSL.KeyFile\n", "", x) ]

certsPath = os.path.abspath(os.path.join(os.getcwd(), "..", "certs"))
keychainPath = os.path.abspath(os.path.join(certsPath, "Find.keychain"))

def cleanup():
    if TestUtil.isDarwin():
        os.system("rm -rf %s ../certs/keychain" % keychainPath)
    elif TestUtil.isLinux():
        for c in ["cacert1.pem", "cacert2.pem"]:
            pem = os.path.join(certsPath, c)
            os.system("rm -f {dir}/`openssl x509 -subject_hash -noout -in {pem}`.0".format(pem=pem, dir=certsPath))

cleanup()
atexit.register(cleanup)

if TestUtil.isDarwin():
    os.system("mkdir -p ../certs/keychain")
    os.system("security create-keychain -p password %s" % keychainPath)
    for cert in ["s_rsa_ca1.p12", "c_rsa_ca1.p12"]:
        os.system("security import %s -f pkcs12 -A -P password -k %s" % (os.path.join(certsPath, cert), keychainPath))
elif TestUtil.iceUseOpenSSL():
    #
    # Create copies of the CA certificates named after the subject
    # hash. This is used by the tests to find the CA certificates in
    # the IceSSL.DefaultDir
    #
    for c in ["cacert1.pem", "cacert2.pem"]:
        pem = os.path.join(certsPath, c)
        os.system("cp {pem} {dir}/`openssl x509 -subject_hash -noout -in {pem}`.0".format(pem=pem, dir=certsPath))

TestUtil.clientServerTest(additionalClientOptions = '"%s"' % os.getcwd())
