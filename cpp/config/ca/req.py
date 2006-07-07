#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os
import sys
import getopt
import shutil
import tempfile

#
# dirname handles finding the .py under Windows since it will contain
# the location of the script. ICE_HOME handles the case where the
# install is in a non-standard location. "." handles
# development. "/usr/bin" handles RPM installs and "/opt/..." handles
# standard tarball installs.
#
for bindir in [os.path.dirname(sys.argv[0]), os.path.join(os.getenv("ICE_HOME"), "bin"), ".", "/usr/bin", \
	       "/opt/Ice-3.1.0/bin"]:
    bindir = os.path.normpath(bindir)
    if os.path.exists(os.path.join(bindir, "cautil.py")):
        break
else:
    raise "can't locate simple CA package"

sys.path.append(os.path.join(bindir))

import cautil

def usage():
    print "usage: " + sys.argv[0] + " [--overwrite] [--node|--registry|--server|--user] [--no-password]"
    sys.exit(1)

def setType(type):
    keyfile = type + "_key.pem"
    reqfile = type + "_req.pem"
    if not overwrite:
	if os.path.exists(keyfile):
	    print keyfile + ": exists"
	    sys.exit(1)
	if os.path.exists(reqfile):
	    print reqfile + ": exists"
	    sys.exit(1)
    return type, keyfile, reqfile

try:
    opts, args = getopt.getopt(sys.argv[1:], "", \
			       [ "overwrite", "node", "registry", "server", "user", "no-password" ])
except getopt.GetoptError:
    usage()

if args:
    usage()

type = None
commonName = None
email = None
nopassphrase = False
overwrite = False
for o, a in opts:
    if o == "--overwrite":
    	overwrite = True
    if o == "--node":
    	if type != None:
    	    usage()
    	    sys.exit(1)
    	type, keyfile, reqfile = setType("node")
	while not commonName or len(commonName) == 0:
	    commonName = raw_input("Enter the node name: ")
    	commonName = "IceGrid Node " + commonName
    elif o == "--registry":
    	if type != None:
    	    usage()
    	    sys.exit(1)
    	type, keyfile, reqfile = setType("registry")
    	commonName = "IceGrid Registry"
    elif o == "--server":
    	if type != None:
    	    usage()
    	    sys.exit(1)
    	type, keyfile, reqfile = setType("server")
	while not commonName or len(commonName) == 0:
	    commonName = raw_input("Enter the server name: ")
    	commonName = "Ice Server " + commonName
    elif o == "--user":
    	if type != None:
    	    usage()
    	    sys.exit(1)
    	type, keyfile, reqfile = setType("user")
	while not commonName or len(commonName) == 0:
	    commonName = raw_input("Enter the user's full name: ")
	while not email or len(email) == 0:
	    email = raw_input("Enter the user's email address: ")
    elif o == "--no-password":
        nopassphrase = True

if not type:
    usage()
    sys.exit(1)

home = cautil.findHome();

#
# Create a temporary configuration file.
#
template = open(os.path.join(home, "req.cnf"), "r")
if not template:
    print "cannot open " + os.path.join(home, "req.cnf")
    sys.exit(1)
data = template.read()
template.close()
temp, tempname = tempfile.mkstemp(".cnf", "req")
os.write(temp, data)
os.write(temp, "commonName=" + commonName + "\n")
if email:
    os.write(temp, "emailAddress=" + email + "\n")
os.close(temp)

cmd = "openssl req -config " + tempname + " -new -keyout '" + keyfile + "' -out '" + reqfile + "'"
if nopassphrase:
    cmd += " -nodes"

#print cmd
status = os.system(cmd)
os.remove(tempname)
if status != 0:
    print "openssl command failed"
    sys.exit(1)

print
print "Created key: " + keyfile
print "Created certificate request: " + reqfile
print
print "The certificate request must be signed by the CA. Send the certificate"
print "request file to the CA at the following email address:"
os.system("openssl x509 -in " + os.path.join(home, "ca_cert.pem") + " -email -noout")
