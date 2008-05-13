#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil, glob

def iceca(args):

    if(os.environ.has_key("ICE_HOME")):
        cmd = os.path.join(os.environ["ICE_HOME"], "bin", "iceca") + " " + args
    else:
        cmd = "iceca " + args

    if os.system(cmd):
        sys.exit(1)

def createCertificate(filename, cn):

    print "======= Creating " + filename + " certificate ======="

    iceca("request --no-password --overwrite %s \"%s\"" % (filename, cn))
    iceca("sign --in %s_req.pem --out %s_cert.pem" % (filename, filename))
    os.remove("%s_req.pem" % filename)

    print
    print

cwd = os.getcwd()

if not os.path.exists("certs") or os.path.basename(cwd) != "secure":
    print "You must run this script from the secure demo directory"
    sys.exit(1)

os.environ["ICE_CA_HOME"] = os.path.abspath("certs")

os.chdir("certs")

#
# First, create the certificate authority.
#
print "======= Creating Certificate Authority ======="
iceca("init --overwrite --no-password")
print
print

createCertificate("registry", "IceGrid Registry")
createCertificate("node", "IceGrid Node")
createCertificate("glacier2", "Glacier2")
createCertificate("server", "Server")

print "======= Creating Java Key Store ======="

try:
    os.remove("certs.jks")
except OSError:
    pass

iceca("import --key-pass password --store-pass password --java ca_cert ca/db/ca_cert.pem ca/db/ca_key.pem certs.jks")

os.chdir("..")
