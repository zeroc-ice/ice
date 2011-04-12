#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, re, getopt

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.abspath(toplevel)
    if os.path.exists(os.path.join(toplevel, "scripts", "TestUtil.py")):
        break
else:
    print "can't find toplevel directory!"
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel))
from scripts import *

#
# List of all basic tests.
#
tests = [ 
    ("IceUtil/condvar", ["once", "win32only"]),
    ("IceUtil/thread",  ["once"]),
    ("IceUtil/unicode", ["once"]),
    ("IceUtil/inputUtil",  ["once"]),
    ("IceUtil/uuid", ["once", "noappverifier"]), # This test is very slow with appverifier.
    ("IceUtil/timer", ["once"]),
    ("IceUtil/fileLock", ["once"]),
    ("Slice/errorDetection", ["once"]),
    ("Slice/keyword", ["once"]),
    ("Slice/structure", ["once"]),
    ("Ice/properties", ["once"]),
    ("Ice/proxy", ["core"]),
    ("Ice/operations", ["core"]),
    ("Ice/exceptions", ["core"]),
    ("Ice/ami", ["core", "novc6"]),
    ("Ice/info", ["core", "noipv6", "nocompress"]),
    ("Ice/inheritance", ["core"]),
    ("Ice/facets", ["core"]),
    ("Ice/objects", ["core"]),
    ("Ice/binding", ["core"]),
    ("Ice/faultTolerance", ["core", "novalgrind"]), # valgrind reports leak with aborted servers
    ("Ice/location", ["core"]),
    ("Ice/adapterDeactivation", ["core"]),
    ("Ice/slicing/exceptions", ["core"]),
    ("Ice/slicing/objects", ["core"]),
    ("Ice/gc", ["once"]),
    ("Ice/dispatcher", ["once"]),
    ("Ice/checksum", ["core"]),
    ("Ice/stream", ["core"]),
    ("Ice/hold", ["core"]),
    ("Ice/custom", ["core", "novc6"]),
    ("Ice/retry", ["core"]),
    ("Ice/timeout", ["core"]),
    ("Ice/background", ["core"]),
    ("Ice/servantLocator", ["core"]),
    ("Ice/interceptor", ["core"]),
    ("Ice/stringConverter", ["core"]),
    ("Ice/udp", ["core"]),
    ("Ice/defaultServant", ["core"]),
    ("Ice/defaultValue", ["core"]),
    ("Ice/invoke", ["core", "novc6"]),
    ("IceSSL/configuration", ["once", "novalgrind"]), # valgrind doesn't work well with openssl
    ("IceBox/configuration", ["core", "noipv6", "nobcc", "novc6"]),
    ("Freeze/dbmap", ["once", "novc6"]),
    ("Freeze/complex", ["once", "novc6"]),
    ("Freeze/evictor", ["core", "novc6"]),
    ("Freeze/fileLock", ["core", "novc6"]),
    ("IceStorm/single", ["service", "novc6", "noappverifier"]),  # This test doensn't work with appverifier is sensitive to timeouts.
    ("IceStorm/federation", ["service", "novc6"]),
    ("IceStorm/federation2", ["service", "novc6"]),
    ("IceStorm/stress", ["service", "stress", "novc6", "noappverifier"]), # This test is very slow with appverifier.
    ("IceStorm/rep1", ["service", "novc6"]),
    ("IceStorm/repgrid", ["service", "novc6"]),
    ("IceStorm/repstress", ["service", "noipv6", "stress", "novc6"]),
    ("FreezeScript/dbmap", ["once", "novc6"]),
    ("FreezeScript/evictor", ["once", "novc6"]),
    ("IceGrid/simple", ["service", "novc6"]),
    ("IceGrid/fileLock", ["service", "nobcc", "novc6"]),
    ("IceGrid/deployer", ["service", "nobcc", "novc6"]),
    ("IceGrid/session", ["service", "nobcc", "novc6"]),
    ("IceGrid/update", ["service", "nobcc", "novc6"]),
    ("IceGrid/activation", ["service", "nobcc", "novc6"]),
    ("IceGrid/replicaGroup", ["service", "nobcc", "novc6"]),
    ("IceGrid/replication", ["service", "nobcc", "novc6"]),
    ("IceGrid/allocation", ["service", "nobcc", "novc6"]),
    ("IceGrid/distribution", ["service", "nobcc", "novc6"]),
    ("IceGrid/admin", ["service", "novc6"]),
    ("Glacier2/router", ["service", "novc6"]),
    ("Glacier2/attack", ["service", "novc6"]),
    ("Glacier2/override", ["service", "novc6"]),
    ("Glacier2/sessionControl", ["service", "novc6"]),
    ("Glacier2/ssl", ["service", "novalgrind", "novc6"]), # valgrind doesn't work well with openssl
    ("Glacier2/dynamicFiltering", ["service", "novc6"]),
    ("Glacier2/staticFiltering", ["service", "noipv6", "novc6"]),
    ]

#
# Run priority tests only if running as root on Unix.
#
if TestUtil.isWin32() or os.getuid() == 0:
    tests += [
        ("IceUtil/priority", ["core", "nodarwin"]),
        ("Ice/threadPoolPriority", ["core", "nodarwin"])
        ]

if __name__ == "__main__":
    TestUtil.run(tests)
