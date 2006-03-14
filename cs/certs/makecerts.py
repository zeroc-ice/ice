#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
    print "-f    Force an update to the C# files."

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

if not os.path.exists("cacert.pem"):
    print sys.argv[0] + ": error: cacert.pem not found"
    sys.exit(1)

update = force
if os.path.exists("cacert.der"):
    cacert_pem_info = os.stat("cacert.pem")
    cacert_der_info = os.stat("cacert.der")
    if cacert_pem_info.st_mtime > cacert_der_info.st_mtime:
	update = 1

if update:
    os.system("openssl x509 -in cacert.pem -outform DER -out cacert.der")
    print "Updated cacert.der"
else:
    print "Skipped cacert.der"

update = force
if not os.path.exists("s_rsa1024.pfx"):
    update = 1
else:
    s_rsa1024_info = os.stat("s_rsa1024.pfx")
    s_rsa1024_pub_info = os.stat("s_rsa1024_pub.pem")
    s_rsa1024_priv_info = os.stat("s_rsa1024_priv.pem")
    if s_rsa1024_pub_info.st_mtime > s_rsa1024_info.st_mtime or \
       s_rsa1024_priv_info.st_mtime > s_rsa1024_info.st_mtime:
       update = 1
    
if update:
    tmp = open("s_rsa1024_pub.pem", "r")
    lines = tmp.readlines()
    tmp.close()
    tmp = open("s_rsa1024_priv.pem", "r")
    lines.extend(tmp.readlines())
    tmp.close()
    tmp = open("s_rsa1024.pem", "w")
    tmp.writelines(lines)
    tmp.close()
    os.system("openssl pkcs12 -in s_rsa1024.pem -export -out s_rsa1024.pfx -passout pass:password")
    os.remove("s_rsa1024.pem")
    print "Updated s_rsa1024.pfx"
else:
    print "Skipped s_rsa1024.pfx"

#
# Done.
#
print "Done."
