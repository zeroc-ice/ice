#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
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
	if status:
	    if(num > 0):
		print "[" + str(num) + "]",
	    print "test in " + dir + " failed with exit status", status,
	    sys.exit(status)

#
# List of all basic tests.
#
tests = [ \
    "Ice/operations", \
    "Ice/exceptions", \
    "Ice/inheritance", \
    "Ice/facets", \
    "Ice/objects", \
    "Ice/faultTolerance", \
    "Ice/location", \
    "Ice/adapterDeactivation", \
    "Ice/slicing/exceptions", \
    "Ice/slicing/objects", \
    "Ice/custom", \
    "Freeze/dbmap", \
    "Freeze/complex", \
    "Freeze/evictor", \
    "Glacier/starter", \
    ]

#
# Certain tests only work on Linux.
#
# The substring on sys.platform is required because some cygwin
# versions return "cygwin_nt-4.01".
#
if sys.platform != "win32" and sys.platform[:6] != "cygwin":
    tests += [ \
        "IcePack/simple", \
        "IcePack/deployer", \
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

loop = False
for o, a in opts:
    if o == "-l":
        loop = True
    
if loop:
    num = 1
    while True:
	runTests(tests, num)
	num += 1
else:
    runTests(tests)
