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
    "IceUtil/thread", \
    "IceUtil/unicode", \
    "IceUtil/inputUtil", \
    "IceUtil/uuid", \
    "Slice/errorDetection", \
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
    "Ice/gc", \
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
    "IcePack/simple", \
    "IcePack/deployer", \
    "Glacier/starter", \
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
