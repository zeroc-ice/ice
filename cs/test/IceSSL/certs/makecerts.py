#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, shutil

for toplevel in [".", "..", "../..", "../../..", "../../../..", "../../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "scripts", "TestUtil.py")):
        break
else:
    raise RuntimeError("can't find toplevel directory!")

sys.path.append(toplevel)
from scripts import *

#
# Show usage information.
#
def usage():
    print("Usage: " + sys.argv[0] + " [options]")
    print("")
    print("Options:")
    print("-h    Show this message.")
    print("-f    Force an update to the C# files.")

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
        print(sys.argv[0] + ": unknown option `" + x + "'")
        print("")
        usage()
        sys.exit(1)
    else:
        usage()
        sys.exit(1)

cppcerts = os.path.join(TestUtil.getIceDir("cpp"), "test", "IceSSL", "certs")

for x in ("cacert1.pem", "cacert2.pem"):
    if force or not os.path.exists(x):
        shutil.copyfile(os.path.join(cppcerts, x), x)

certs = [\
    "c_rsa_nopass_ca1_exp", \
    "c_rsa_nopass_ca1", \
    "c_rsa_nopass_ca2", \
    "s_rsa_nopass_ca1_exp", \
    "s_rsa_nopass_ca1", \
    "s_rsa_nopass_ca2", \
    "s_rsa_nopass_ca1_cn1", \
    "s_rsa_nopass_ca1_cn2", \
]

for x in certs:
    if force or not os.path.exists(x + ".pfx"):
        cert = os.path.join(cppcerts, x)
        os.system("openssl pkcs12 -in " + cert + "_pub.pem -inkey " + cert + "_priv.pem -export -out " + x + \
                  ".pfx -passout pass:password")
        print("Created " + x + ".pfx")

#
# Done.
#
print("Done.")
