#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    "IceUtil/thread", 
    "IceUtil/unicode",
    "IceUtil/inputUtil", 
    "IceUtil/uuid",
    "IceUtil/timer",
    "Slice/errorDetection",
    "Slice/keyword",
    "Ice/proxy",
    "Ice/operations",
    "Ice/exceptions",
    "Ice/inheritance",
    "Ice/facets",
    "Ice/objects",
    "Ice/binding",
    "Ice/faultTolerance",
    "Ice/location",
    "Ice/adapterDeactivation",
    "Ice/slicing/exceptions",
    "Ice/slicing/objects",
    "Ice/gc",
    "Ice/checksum",
    "Ice/stream",
    "Ice/hold",
    "Ice/custom",
    "Ice/retry",
    "Ice/timeout",
    "Ice/background",
    "Ice/servantLocator",
    "Ice/threads",
    "Ice/interceptor",
    "Ice/stringConverter",
    "IceSSL/configuration",
    "Freeze/dbmap",
    "Freeze/complex",
    "Freeze/evictor",
    "Freeze/oldevictor",
    "IceStorm/single",
    "IceStorm/federation",
    "IceStorm/federation2",
    "IceStorm/stress",
    "FreezeScript/dbmap",
    "FreezeScript/evictor",
    "IceGrid/simple",
    "IceGrid/deployer",
    "IceGrid/session",
    "IceGrid/update",
    "IceGrid/activation",
    "IceGrid/replicaGroup",
    "IceGrid/replication",
    "IceGrid/allocation",
    "IceGrid/distribution",
    "Glacier2/router",
    "Glacier2/attack",
    "Glacier2/sessionControl",
    "Glacier2/ssl",
    "Glacier2/dynamicFiltering",
    "Glacier2/staticFiltering",
    ]

#
# These tests are currently disabled on cygwin
#
if TestUtil.isCygwin() == 0:
    tests += [
      ]

if TestUtil.isWin32():
    tests.insert(0, "IceUtil/condvar")

if __name__ == "__main__":
    TestUtil.run(tests)
