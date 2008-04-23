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

def createCertificate(type, filename = None):

    if not filename:
        filename = type

    print "======= Creating " + filename + " certificate ======="

    # Generate the certificate in a temporary directory
    os.mkdir("tmpcerts")
    os.chdir("tmpcerts")
    iceca("request --" + type + nopassword)
    iceca("sign --in " + type + "_req.pem --out " + type + "_cert.pem")
    os.chdir("..")

    # Move and rename the generated certificate
    os.rename(os.path.join("tmpcerts", type + "_key.pem"), filename + "_key.pem")
    os.rename(os.path.join("tmpcerts", type + "_cert.pem"), filename + "_cert.pem")
    
    # Remove the temporary directory
    os.remove(os.path.join("tmpcerts", type + "_req.pem"))
    os.rmdir("tmpcerts")

    print
    print

cwd = os.getcwd()

if not os.path.exists("certs") or os.path.basename(cwd) != "secure":
    print "You must run this script from the secure demo directory"
    sys.exit(1)

os.environ["ICE_CA_HOME"] = os.path.abspath("certs")
nopassword = " --no-password"

os.chdir("certs")

#
# First, create the certificate authority.
#
print "======= Creating Certificate Authority ======="
iceca("init --overwrite" + nopassword)
print
print

createCertificate("registry")
createCertificate("node")
createCertificate("server", "glacier2")
createCertificate("server")
createCertificate("server", "admin")

print "======= Creating Java Key Store ======="

iceca("import --java admin admin_cert.pem admin_key.pem certs.jks")

os.chdir("..")
