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
    print "usage: " + sys.argv[0] + " --in <req> --out <cert>"
    sys.exit(1)

try:
    opts, args = getopt.getopt(sys.argv[1:], "", [ "in=", "out=" ])
except getopt.GetoptError:
    usage()

if args:
    usage()

infile = None
outfile = None
for o, a in opts:
    if o == "--in":
    	infile = a
    elif o == "--out":
	outfile = a

if infile == None or outfile == None:
    usage()

home = cautil.findHome();
caroot = os.path.join(home, "ca")
cadb = os.path.join(caroot, "db")

cmd = "openssl ca -config " + os.path.join(caroot, "sign.cnf") + " -in " + infile + " -out " + outfile
status = os.system(cmd)
if status != 0:
    print "openssl command failed"
    sys.exit(1)
