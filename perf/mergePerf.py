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
    print "usage: " + sys.argv[0] + " [-h|--help] [-o|-output FILENAME] [-r|--remove EXPR] FILENAME ..."
    print ""
    print "Merge the results from different file into a single file. If no output"
    print "file is specified, the name of the file used to create the results will"
    print "be used"
    print
    print "Command line options:"
    print ""
    print " -h | --help           Print this help message."
    print " -o | -output          Defines the name of the output file."
    print " -r | -remove          Remove some tests."
    print ""
    sys.exit(2)

try:
    opts, pargs = getopt.getopt(sys.argv[1:], 'ho:r:', ['help', 'output=', 'remove=']);
except getopt.GetoptError:
    usage()

if len(pargs) < 1:
    usage()

outputFile = ""
removeExpr = ""
for o, a in opts:
    if o == '-h' or o == "--help":
        usage()
    elif o == '-o' or o == "--output":
        outputFile = a
    elif o == '-r' or o == "--remove":
        removeExpr = a

all = TestUtil.AllResults()
for fname in pargs:
    f = file(fname)
    all.add(pickle.load(f))
    f.close

if removeExpr:
    all.remove(re.compile(removeExpr))

all.saveAll(outputFile)
