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
import tempfile
import getpass

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
    print "usage: " + sys.argv[0] + " [--overwrite] [--java alias cert key keystore] [--cs cert key out-file]"
    sys.exit(1)

try:
    opts, args = getopt.getopt(sys.argv[1:], "", [ "overwrite", "java", "cs"])
except getopt.GetoptError:
    usage()

home = cautil.findHome();

java = False
cs = False
overwrite = False
for o, a in opts:
    if o == "--overwrite":
    	overwrite = True
    if o == "--java":
    	java = True
    if o == "--cs":
    	cs = True

if not java and not cs:
    print sys.argv[0] + ": one of --java or --cs must be provided"
    usage()

if java:
    if len(args) != 4:
    	usage()

    alias = args[0]
    cert = args[1]
    key = args[2]
    store = args[3]

    keyPass = getpass.getpass("Enter private key passphrase:")

    storePass = ""
    while len(storePass) == 0:
	storePass = getpass.getpass("Enter keystore password:")

    #
    # We use secure temporary files to transfer the password to openssl
    # and the ImportKey java tool. Its necessary to create 2 files for the
    # key password because openssl craps out if you use the same password
    # file twice.
    #
    keypassfile1 = None
    keypassfile2 = None

    if len(keyPass) > 0:
	temp, keypassfile1 = tempfile.mkstemp("keypass1")
	os.write(temp, keyPass)
	os.close(temp)

	temp, keypassfile2 = tempfile.mkstemp("keypass2")
	os.write(temp, keyPass)
	os.close(temp)
    else:
	# Java can't deal with unencrypted keystores, so we use a
	# temporary one.
	temp, keypassfile1 = tempfile.mkstemp("keypass1")
	os.write(temp, "password")
	os.close(temp)

	# We create a file with an empty password to store the results
	# in the keystore.
	temp, keypassfile2 = tempfile.mkstemp("keypass2")
	os.close(temp)

    temp, storepassfile = tempfile.mkstemp("storepass3")
    os.write(temp, storePass)
    os.close(temp)

    temp, pkcs12cert = tempfile.mkstemp(".p12", "pkcs12")
    os.close(temp)

    if len(keyPass) > 0:
	cmd = "openssl pkcs12 -in " + cert + " -inkey " + key + " -export -out " + pkcs12cert + " -name " + alias + \
	    " -passin file:" + keypassfile1 + " -passout file:" + keypassfile2 + " -certfile " + \
	    os.path.join(home, "ca_cert.pem")
    else:
	cmd = "openssl pkcs12 -in " + cert + " -inkey " + key + " -export -out " + pkcs12cert + " -name " + alias + \
	    " -passout file:" + keypassfile1 + " -certfile " + os.path.join(home, "ca_cert.pem")

    #print cmd
    print "converting to pkcs12 format... ",
    status = os.system(cmd)
    if status != 0:
	print "openssl command failed"
	os.remove(keypassfile1)
	os.remove(keypassfile2)
	os.remove(storepassfile)
	sys.exit(1)
    print "ok"

    # Use java to import the cert into the keystore.
    cmd = "java -classpath " + bindir + " ImportKey " + pkcs12cert + " " + alias + " " \
	      + os.path.join(home, "ca_cert.pem") + " " + store + " " + storepassfile + " " + keypassfile1
    if len(keyPass) == 0:
    	cmd = cmd + " " + keypassfile2

    #print cmd
    print "importing into the keystore...", 
    status = os.system(cmd)
    if status != 0:
	print "java command failed"
    else:
	print "ok"

    # Cleanup.
    os.remove(pkcs12cert)
    os.remove(keypassfile1)
    os.remove(keypassfile2)
    os.remove(storepassfile)

if cs:
    if len(args) != 3:
    	usage()

    cert = args[0]
    key = args[1]
    pkcs12cert = args[2]
    if not overwrite and os.path.exists(pkcs12cert):
    	print pkcs12cert + ": file exists"
    	sys.exit(1)

    keyPass = getpass.getpass("Enter private key passphrase:")

    #
    # We use secure temporary files to transfer the password to
    # openssl Its necessary to create 2 files for the key password
    # because openssl craps out if you use the same password file
    # twice.
    #
    keypassfile1 = None
    keypassfile2 = None

    if len(keyPass) > 0:
	temp, keypassfile1 = tempfile.mkstemp("keypass1")
	os.write(temp, keyPass)
	os.close(temp)

	temp, keypassfile2 = tempfile.mkstemp("keypass2")
	os.write(temp, keyPass)
	os.close(temp)

	cmd = "openssl pkcs12 -in " + cert + " -inkey " + key + " -export -out " + pkcs12cert + \
	    " -passin file:" + keypassfile1 + " -passout file:" + keypassfile2
    else:
	cmd = "openssl pkcs12 -in " + cert + " -inkey " + key + " -export -out " + pkcs12cert + " -passout pass:"

    #print cmd
    print "converting to pkcs12 format...",
    status = os.system(cmd)
    if keypassfile1 != None:
	os.remove(keypassfile1)
    if keypassfile2 != None:
	os.remove(keypassfile2)
    if status != 0:
	print "openssl command failed"
	sys.exit(1)
    print "ok"
