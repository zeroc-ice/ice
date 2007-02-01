#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil

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

if not os.environ.has_key("ICE_HOME"):
    print sys.argv[0] + ": error: ICE_HOME is not defined"
    sys.exit(1)

#
# Convert OpenSSL key/certificate pairs into PKCS12 format and then
# import them into a Java keystore.
#
target = "client.jks"
if force or not os.path.exists(target):
    if os.path.exists(target):
        os.remove(target)
    os.system("openssl pkcs12 -in c_rsa1024_pub.pem -inkey c_rsa1024_priv.pem -export -out client.p12" \
              " -name rsakey -passout pass:password -certfile cacert.pem")
    os.system("java -classpath . ImportKey client.p12 rsakey cacert.pem " + target + " password")
    os.remove("client.p12")
    print "Created " + target
target = "server.jks"
if force or not os.path.exists(target):
    if os.path.exists(target):
        os.remove(target)
    os.system("openssl pkcs12 -in s_rsa1024_pub.pem -inkey s_rsa1024_priv.pem -export -out server.p12" \
              " -name rsakey -passout pass:password -certfile cacert.pem")
    os.system("java -classpath . ImportKey server.p12 rsakey cacert.pem " + target + " password")
    os.remove("server.p12")
    print "Created " + target

#
# Create a truststore from the CA certificate.
#
ts = "certs.jks"
if force or not os.path.exists(ts):
    if os.path.exists(ts):
        os.remove(ts)
    os.system("openssl x509 -in cacert.pem -outform DER -out cacert.der")
    os.system("keytool -import -alias cacert -file cacert.der -keystore " + ts + \
              " -storepass password -noprompt")
    os.remove("cacert.der")
    print "Created " + ts

#
# Done.
#
print "Done."
