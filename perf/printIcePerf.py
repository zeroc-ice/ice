#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, time, pickle, getopt, re

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

def usage():
    print "usage: " + sys.argv[0] + " [-h|--help] [-n|-hostname HOSTNAME] FILENAME ..."
    print ""
    print "Print the results stored in the given files."
    print ""
    print "Command line options:"
    print ""
    print " -h | --help           Print this help message."
    print " -n | --hostname       Print this help message."
    print ""
    sys.exit(2)

try:
    opts, pargs = getopt.getopt(sys.argv[1:], 'hn:', ['help', 'hostname=']);
except getopt.GetoptError:
    usage()

niter = max
printResults = False
hostname = ""
outputFile = ""
for o, a in opts:
    if o == '-h' or o == "--help":
        usage()
    elif o == '-n' or o == "--hostname":
        hostname = a

if hostname == "":
    import socket
    hostname = socket.gethostname()
outputFile = "results." + sys.platform + "." + hostname

all = TestUtil.AllResults()
if len(pargs) > 0:
    for fname in pargs:
        f = file(fname)
        all.add(pickle.load(f))
        f.close
else:
    f = file(outputFile)
    all.add(pickle.load(f));
    f.close()
            
all.printAll()
