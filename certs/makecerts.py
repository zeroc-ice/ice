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
if not os.path.exists("c_rsa1024.pfx"):
    update = 1
else:
    c_rsa1024_info = os.stat("c_rsa1024.pfx")
    c_rsa1024_pub_info = os.stat("c_rsa1024_pub.pem")
    c_rsa1024_priv_info = os.stat("c_rsa1024_priv.pem")
    if c_rsa1024_pub_info.st_mtime > c_rsa1024_info.st_mtime or \
       c_rsa1024_priv_info.st_mtime > c_rsa1024_info.st_mtime:
       update = 1
    
if update:
    os.system("openssl pkcs12 -in c_rsa1024_pub.pem -inkey c_rsa1024_priv.pem -export -out c_rsa1024.pfx -passout pass:password")
    print "Updated c_rsa1024.pfx"
else:
    print "Skipped c_rsa1024.pfx"

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
    os.system("openssl pkcs12 -in s_rsa1024_pub.pem -inkey s_rsa1024_priv.pem -export -out s_rsa1024.pfx -passout pass:password")
    print "Updated s_rsa1024.pfx"
else:
    print "Skipped s_rsa1024.pfx"

#
# Done.
#
print "Done."
