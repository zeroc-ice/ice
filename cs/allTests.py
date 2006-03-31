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

def runTests(mono, tests, num = 0):

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

	if mono:
	    status = os.system(os.path.join(dir, "run.py -m"))
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
    "IceUtil/inputUtil", \
    "Ice/operations", \
    "Ice/exceptions", \
    "Ice/inheritance", \
    "Ice/facets", \
    "Ice/hold", \
    "Ice/objects", \
    "Ice/binding", \
    "Ice/faultTolerance", \
    "Ice/location", \
    "Ice/adapterDeactivation", \
    "Ice/slicing/exceptions", \
    "Ice/slicing/objects", \
    "Ice/checksum", \
    "Ice/stream", \
    "Ice/retry", \
    "Glacier2/router", \
    "IceGrid/simple", \
    ]
if os.path.exists(os.path.join(toplevel, "bin", "icesslcs.dll")):
    tests.append("IceSSL/configuration")

def usage():
    print "usage: " + sys.argv[0] + " [-l]"
    sys.exit(2)

try:
    opts, args = getopt.getopt(sys.argv[1:], "lm")
except getopt.GetoptError:
    usage()

if(args):
    usage()

loop = 0
mono = 0
for o, a in opts:
    if o == "-l":
        loop = 1
    if o == "-m":
        mono = 1
    
if not TestUtil.isWin32():
    mono = 1

if loop:
    num = 1
    while 1:
	runTests(mono, tests, num)
	num += 1
else:
    runTests(mono, tests)
