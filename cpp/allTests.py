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

#
# I must import "time" here, otherwise I get a "NameError: global name
# 'time' is not defined" in the IceStorm run.py scripts. I have no
# idea why.
#
import time

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
    "Glacier/starter", \
    "IcePack/simple", \
    ]

#
# TODO: find out why the deployer test doesn't work on Windows.
#
if TestUtil.isWin32() == 0:
    tests += [ \
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
