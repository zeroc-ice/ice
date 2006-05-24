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

        status = os.system(os.path.join(dir, "run.py"))

	if status and not (sys.platform.startswith("aix") and status == 256):
	    if(num > 0):
		print "[" + str(num) + "]",
	    print "test in " + dir + " failed with exit status", status,
	    sys.exit(status)

#
# List of all basic tests.
#
tests = [ \
    "Ice/adapterDeactivation", \
    "Ice/binding", \
    "Ice/exceptions", \
    "Ice/facets", \
    "Ice/faultTolerance", \
    "Ice/inheritance", \
    "Ice/location", \
    "Ice/objects", \
    "Ice/operations", \
    "Ice/slicing/exceptions", \
    "Ice/slicing/objects", \
    "Ice/checksum", \
    "Ice/retry", \
    ]

def usage():
    print "usage: " + sys.argv[0] + " [-l][-r <regex>]"
    sys.exit(2)

try:
    opts, args = getopt.getopt(sys.argv[1:], "lr:")
except getopt.GetoptError:
    usage()

if(args):
    usage()

loop = 0
for o, a in opts:
    if o == "-l":
        loop = 1
    if o == "-r":
        import re
        regexp = re.compile(a)
        newtests = []
        def rematch(x): return regexp.match(x)
        tests = filter(rematch, tests)
    
if loop:
    num = 1
    while 1:
	runTests(tests, num)
	num += 1
else:
    runTests(tests)
