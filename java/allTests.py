#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2002
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

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

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
    "Ice/locationForward", \
    "Ice/location", \
    "Ice/adapterDeactivation", \
    "IceXML/encoding", \
    "Freeze/cursor", \
    "Freeze/dbmap", \
    "Freeze/complex", \
    "Freeze/evictor", \
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
        "Glacier/starter", \
        ]

#
# The user can supply a subset of tests on the command line.
#
if sys.argv[1:]:
    print "limiting tests"
    newtests = []
    for i in tests:
	if i in sys.argv[1:]:
	    newtests.append(i)
    tests = newtests

#
# Run each of the tests.
#
for i in tests:

    i = os.path.normpath(i)
    dir = os.path.join(toplevel, "test", i)

    print
    print "*** running tests in " + dir + ":"
    print

    try:
        execfile(os.path.join(dir, "run.py"))
    except SystemExit, (status,):
        if status:
            print "test failed with exit status", status
            sys.exit(status)
