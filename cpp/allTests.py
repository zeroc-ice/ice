#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, re, getopt

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")

sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

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
    ("Ice/properties", ["once", "novc6", "nomingw"]),
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
    ("Ice/custom", ["core", "novc6", "nossl"]),
    ("Ice/retry", ["core"]),
    ("Ice/timeout", ["core"]),
    ("Ice/background", ["core", "nomingw"]),
    ("Ice/servantLocator", ["core"]),
    ("Ice/interceptor", ["core"]),
    ("Ice/stringConverter", ["core"]),
    ("Ice/udp", ["core"]),
    ("Ice/defaultServant", ["core"]),
    ("Ice/defaultValue", ["core"]),
    ("Ice/invoke", ["core", "novc6"]),
    ("Ice/plugin", ["core"]),
    ("Ice/hash", ["once"]),
    ("Ice/admin", ["core"]),
    ("Ice/metrics", ["core"]),
    ("IceSSL/configuration", ["once", "novalgrind"]), # valgrind doesn't work well with openssl
    ("IceBox/configuration", ["core", "noipv6", "novc6", "nomingw"]),
    ("IceBox/admin", ["core", "noipv6", "novc6", "nomingw"]),
    ("Freeze/dbmap", ["once", "novc6", "nomingw"]),
    ("Freeze/complex", ["once", "novc6", "nomingw"]),
    ("Freeze/evictor", ["core", "novc6", "nomingw"]),
    ("Freeze/fileLock", ["core", "novc6", "nomingw"]),
    ("IceStorm/single", ["service", "novc6", "noappverifier", "nomingw"]),  # This test doesn't work with appverifier is sensitive to timeouts.
    ("IceStorm/federation", ["service", "novc6", "nomingw"]),
    ("IceStorm/federation2", ["service", "novc6", "nomingw"]),
    ("IceStorm/stress", ["service", "stress", "novc6", "noappverifier", "nomingw"]), # This test is very slow with appverifier.
    ("IceStorm/rep1", ["service", "novc6", "nomingw"]),
    ("IceStorm/repgrid", ["service", "novc6", "nomingw"]),
    ("IceStorm/repstress", ["service", "noipv6", "stress", "novc6", "nomingw"]),
    ("FreezeScript/dbmap", ["once", "novc6", "nomingw"]),
    ("FreezeScript/evictor", ["once", "novc6", "nomingw"]),
    ("IceGrid/simple", ["service", "novc6", "nomingw"]),
    ("IceGrid/fileLock", ["service", "novc6", "nomingw"]),
    ("IceGrid/deployer", ["service", "novc6", "nomingw"]),
    ("IceGrid/session", ["service", "novc6", "nomingw"]),
    ("IceGrid/update", ["service", "novc6", "nomingw"]),
    ("IceGrid/noRestartUpdate", ["service", "novc6", "nomingw"]),
    ("IceGrid/activation", ["service", "novc6", "nomingw"]),
    ("IceGrid/replicaGroup", ["service", "novc6", "nomingw"]),
    ("IceGrid/replication", ["service", "novc6", "nomingw"]),
    ("IceGrid/allocation", ["service", "novc6", "nomingw"]),
    ("IceGrid/distribution", ["service", "novc6", "nomingw"]),
    ("IceGrid/admin", ["service", "novc6", "nomingw"]),
    ("Glacier2/router", ["service", "novc6", "nomingw"]),
    ("Glacier2/attack", ["service", "novc6", "nomingw"]),
    ("Glacier2/override", ["service", "novc6", "nomingw"]),
    ("Glacier2/sessionControl", ["service", "novc6", "nomingw"]),
    ("Glacier2/ssl", ["service", "novalgrind", "novc6", "nomingw"]), # valgrind doesn't work well with openssl
    ("Glacier2/dynamicFiltering", ["service", "novc6", "nomingw"]),
    ("Glacier2/staticFiltering", ["service", "noipv6", "novc6", "nomingw"]),
    ("Glacier2/sessionHelper", ["service", "novc6", "nomingw"]),
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
