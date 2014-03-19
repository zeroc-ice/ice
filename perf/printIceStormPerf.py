#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, time, pickle, getopt, re, platform

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise RuntimeError("can't find toplevel directory!")

sys.path.append(os.path.join(toplevel, "config"))
from scripts import *

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
    opts, pargs = getopt.getopt(sys.argv[1:], 'hn:c', ['help', 'hostname=', 'csv', 'csv2']);
except getopt.GetoptError:
    usage()

niter = max
printResults = False
hostname = ""
filename = ""
outputFormat = "text"
for o, a in opts:
    if o == '-h' or o == "--help":
        usage()
    elif o == '-n' or o == "--hostname":
        hostname = a

(system, name, ver, build, machine, processor) = platform.uname()
if hostname == "":
    hostname = name
    if hostname.find('.'):
        hostname = hostname[0:hostname.find('.')]
filename = ("results.icestorm." + system + "." + hostname).lower()

inputfile = file(filename)
rawResults = eval(inputfile.read())
inputfile.close()

TestUtil.PrintResults(rawResults, filename, [('IceStorm', 'CosEvent')])

