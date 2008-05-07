#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, re, getopt

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.abspath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

#
# List of all basic tests.
#
tests = [ 
    ("IceUtil/thread",  True),
    ("IceUtil/unicode", True),
    ("IceUtil/inputUtil",  True),
    ("IceUtil/uuid", True),
    ("IceUtil/timer", True),
    ("Slice/errorDetection", True),
    ("Slice/keyword", True),
    ("Ice/proxy", False),
    ("Ice/operations", False),
    ("Ice/exceptions", False),
    ("Ice/inheritance", False),
    ("Ice/facets", False),
    ("Ice/objects", False),
    ("Ice/binding", False),
    ("Ice/faultTolerance", False),
    ("Ice/location", False),
    ("Ice/adapterDeactivation", False),
    ("Ice/slicing/exceptions", False),
    ("Ice/slicing/objects", False),
    ("Ice/gc", False),
    ("Ice/checksum", False),
    ("Ice/stream", False),
    ("Ice/hold", False),
    ("Ice/custom", False),
    ("Ice/retry", False),
    ("Ice/timeout", False),
    ("Ice/background", False),
    ("Ice/servantLocator", False),
    ("Ice/interceptor", False),
    ("Ice/stringConverter", False),
    ("IceSSL/configuration", False),
    ("IceBox/configuration", False),
    ("Freeze/dbmap", True),
    ("Freeze/complex", True),
    ("Freeze/evictor", False),
    ("Freeze/oldevictor", False),
    ("IceStorm/single", False),
    ("IceStorm/federation", False),
    ("IceStorm/federation2", False),
    ("IceStorm/stress", False),
    ("IceStorm/rep1", False),
    ("IceStorm/repgrid", False),
    ("IceStorm/repstress", False),
    ("FreezeScript/dbmap", True),
    ("FreezeScript/evictor", True),
    ("IceGrid/simple", False),
    ("IceGrid/deployer", False),
    ("IceGrid/session", False),
    ("IceGrid/update", False),
    ("IceGrid/activation", False),
    ("IceGrid/replicaGroup", False),
    ("IceGrid/replication", False),
    ("IceGrid/allocation", False),
    ("IceGrid/distribution", False),
    ("Glacier2/router", False),
    ("Glacier2/attack", False),
    ("Glacier2/sessionControl", False),
    ("Glacier2/ssl", False),
    ("Glacier2/dynamicFiltering", False),
    ("Glacier2/staticFiltering", False),
    ]

#
# These tests are currently disabled on cygwin
#
if TestUtil.isCygwin() == 0:
    tests += [
      ]

if TestUtil.isWin32():
    tests.insert(0, ("IceUtil/condvar", True))

if __name__ == "__main__":
    TestUtil.run(tests)
