#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
#
# **********************************************************************

import os, sys

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

#
# List of all tests.
#
tests = [ \
    "Slice/errorDetection", \
    "Ice/operations", \
    "Ice/exceptions", \
    "Ice/inheritance", \
    "Ice/facets", \
    "Ice/objects", \
    "Ice/faultTolerance", \
    "Ice/locationForward", \
    "IcePack/simple", \
    "Freeze/cursor", \
    "Freeze/dbmap", \
    "Freeze/complex", \
    ]

#
# The user can supply on the command line a subset of tests to run.
#
if sys.argv[1:]:
    print "limiting tests"
    newtests = []
    for i in tests:
	if i in sys.argv[1:]:
	    newtests.append(i)
    tests = newtests

import string

#
# Run each of the tests.
#
for i in tests:
    dir = os.path.join(toplevel, "test")

    #
    # Construct the test path
    #
    pathElements = os.path.join(string.split(i, "/"))
    for element in pathElements:
        dir = os.path.join(dir, element)

    print
    print "*** running tests in " + dir + ":"
    print

    try:
        execfile(os.path.join(dir, "run.py"))
    except SystemExit, (status,):
        if status:
            sys.exit(status)
