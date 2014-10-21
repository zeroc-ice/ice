#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, atexit

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

certsPath = os.path.abspath(os.path.join(os.getcwd(), "..", "certs"))
keychainPath = os.path.abspath(os.path.join(certsPath, "Find.keychain"))


def keychainCleanup():
    os.system("rm -rf %s ../certs/keychain" % keychainPath)

if TestUtil.isDarwin():
    atexit.register(keychainCleanup)
    keychainCleanup()
    os.system("mkdir -p ../certs/keychain")

    os.system("security create-keychain -p password %s" % keychainPath)
    for cert in ["s_rsa_ca1.pfx", "c_rsa_ca1.pfx"]:
        os.system("security import %s -f pkcs12 -A -P password -k %s" % (os.path.join(certsPath, cert), keychainPath))

TestUtil.clientServerTest(additionalClientOptions = '"%s"' % os.getcwd())
