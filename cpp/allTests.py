#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys
import getopt

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

def runTests(tests, num = 0):

    #
    # Run each of the tests.
    #
    for i in tests:

	i = os.path.normpath(i)
	dir = os.path.join(toplevel, "test", i)

	print
	if(num > 0):
	    print "[" + str(num) + "]",
	print "*** running tests in " + dir,
	print

        if TestUtil.isWin9x():
	    status = os.system("python " + os.path.join(dir, "run.py"))
        else:
            status = os.system(os.path.join(dir, "run.py"))

	if status:
	    if(num > 0):
		print "[" + str(num) + "]",
	    print "test in " + dir + " failed with exit status", status,
	    sys.exit(status)

#
# List of all basic tests.
#
tests = [ \
    "IceUtil/thread", \
#    "IceUtil/unicode", \
    "IceUtil/inputUtil", \
    "IceUtil/uuid", \
    "Slice/errorDetection", \
    "Ice/operations", \
    "Ice/exceptions", \
    "Ice/inheritance", \
    "Ice/facets", \
    "Ice/objects", \
    "Ice/binding", \
    "Ice/faultTolerance", \
    "Ice/location", \
    "Ice/adapterDeactivation", \
    "Ice/slicing/exceptions", \
    "Ice/slicing/objects", \
    "Ice/gc", \
    "Ice/checksum", \
    "Ice/stream", \
    "Ice/hold", \
    "Ice/custom", \
    "Ice/retry", \
    "IceSSL/configuration", \
    "IceSSL/loadPEM", \
    "IceSSL/certificateAndKeyParsing", \
    "IceSSL/certificateVerifier", \
    "IceSSL/certificateVerification", \
    "Freeze/dbmap", \
    "Freeze/complex", \
    "Freeze/evictor", \
    "IceStorm/single", \
    "IceStorm/federation", \
    "IceStorm/federation2", \
    "FreezeScript/dbmap", \
    "FreezeScript/evictor", \
    "IceGrid/simple", \
    "IceGrid/deployer", \
    "IceGrid/session", \
    "IceGrid/update", \
    "IceGrid/activation", \
    "IceGrid/replication", \
    "Glacier2/router", \
    ]

#
# These tests are currently disabled on cygwin
#
if TestUtil.isCygwin() == 0:
    tests += [ \
       
      ]

def usage():
    print "usage: " + sys.argv[0] + " [-l]"
    sys.exit(2)

try:
    opts, args = getopt.getopt(sys.argv[1:], "l")
except getopt.GetoptError:
    usage()

if(args):
    usage()

loop = 0
for o, a in opts:
    if o == "-l":
        loop = 1
    
if loop:
    num = 1
    while 1:
	runTests(tests, num)
	num += 1
else:
    runTests(tests)
