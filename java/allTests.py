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
import optparse

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

	try:
	    execfile(os.path.join(dir, "run.py"))
	except SystemExit, (status,):
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

parser = optparse.OptionParser()
parser.add_option("-l", "--loop", action="store_true", dest="loop", default=False,
		  help="run tests continously in an endless loop")
(options, args) = parser.parse_args()

if options.loop:
    num = 1
    while 1:
	runTests(tests, num)
	num += 1
else:
    runTests(tests)
