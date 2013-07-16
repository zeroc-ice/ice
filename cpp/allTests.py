#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
    ("Slice/macros", ["once"]),
    ("Ice/properties", ["once", "nomingw"]),
    ("Ice/proxy", ["core"]),
    ("Ice/operations", ["core"]),
    ("Ice/exceptions", ["core"]),
    ("Ice/ami", ["core"]),
    ("Ice/info", ["core", "noipv6", "nocompress", "nosocks"]),
    ("Ice/inheritance", ["core"]),
    ("Ice/facets", ["core"]),
    ("Ice/objects", ["core"]),
    ("Ice/optional", ["core"]),
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
    ("Ice/custom", ["core", "nossl"]),
    ("Ice/retry", ["core"]),
    ("Ice/timeout", ["core"]),
    ("Ice/background", ["core", "nomingw"]),
    ("Ice/servantLocator", ["core"]),
    ("Ice/interceptor", ["core"]),
    ("Ice/stringConverter", ["core"]),
    ("Ice/udp", ["core"]),
    ("Ice/defaultServant", ["core"]),
    ("Ice/defaultValue", ["core"]),
    ("Ice/invoke", ["core"]),
    ("Ice/plugin", ["core", "nomingw"]),
    ("Ice/hash", ["once"]),
    ("Ice/admin", ["core", "noipv6"]),
    ("Ice/metrics", ["core", "nossl", "noipv6", "nocompress", "nomingw", "nosocks"]),
    ("Ice/enums", ["once"]),
    ("IceSSL/configuration", ["once", "novalgrind"]), # valgrind doesn't work well with openssl
    ("IceBox/configuration", ["core", "noipv6", "novc90", "nomingw", "nomx"]),
    ("IceBox/admin", ["core", "noipv6", "novc90", "nomingw", "nomx"]),
    ("Freeze/dbmap", ["once", "novc90", "nomingw"]),
    ("Freeze/complex", ["once", "novc90", "nomingw"]),
    ("Freeze/evictor", ["core", "novc90", "nomingw"]),
    ("Freeze/fileLock", ["core", "novc90", "nomingw"]),
    ("IceStorm/single", ["service", "novc90", "noappverifier", "nomingw"]), # This test doesn't work with appverifier
    ("IceStorm/federation", ["service", "novc90", "nomingw"]),
    ("IceStorm/federation2", ["service", "novc90", "nomingw"]),
    ("IceStorm/stress", ["service", "stress", "novc90", "noappverifier", "nomingw"]), # Too slow with appverifier.
    ("IceStorm/rep1", ["service", "novc90", "nomingw"]),
    ("IceStorm/repgrid", ["service", "novc90", "nomingw"]),
    ("IceStorm/repstress", ["service", "noipv6", "stress", "novc90", "nomingw"]),
    ("FreezeScript/dbmap", ["once", "novc90", "nomingw"]),
    ("FreezeScript/evictor", ["once", "novc90", "nomingw"]),
    ("IceGrid/simple", ["service", "novc90", "nomingw"]),
    ("IceGrid/fileLock", ["service", "novc90", "nomingw"]),
    ("IceGrid/deployer", ["service", "novc90", "nomingw"]),
    ("IceGrid/session", ["service", "novc90", "nomingw"]),
    ("IceGrid/update", ["service", "novc90", "nomingw"]),
    ("IceGrid/noRestartUpdate", ["service", "novc90", "nomingw"]),
    ("IceGrid/activation", ["service", "novc90", "nomingw"]),
    ("IceGrid/replicaGroup", ["service", "novc90", "nomingw"]),
    ("IceGrid/replication", ["service", "novc90", "nomingw", "nomx"]),
    ("IceGrid/allocation", ["service", "novc90", "nomingw"]),
    ("IceGrid/distribution", ["service", "novc90", "nomingw"]),
    ("IceGrid/admin", ["service", "novc90", "nomingw"]),
    ("Glacier2/router", ["service", "novc90", "nomingw"]),
    ("Glacier2/attack", ["service", "novc90", "nomingw", "nomx"]),
    ("Glacier2/override", ["service", "novc90", "nomingw"]),
    ("Glacier2/sessionControl", ["service", "novc90", "nomingw"]),
    ("Glacier2/ssl", ["service", "novalgrind", "novc90", "nomingw"]), # valgrind doesn't work well with openssl
    ("Glacier2/dynamicFiltering", ["service", "novc90", "nomingw"]),
    ("Glacier2/staticFiltering", ["service", "noipv6", "novc90", "nomingw", "nomx"]),
    ("Glacier2/sessionHelper", ["service", "novc90", "nomingw", "nossl", "noipv6"]),
    ]

#
# Run priority tests only if running as root on Unix.
#
if TestUtil.isWin32() or os.getuid() == 0:
    tests += [
        ("IceUtil/priority", ["core", "nodarwin"]),
        ("Ice/threadPoolPriority", ["core", "nodarwin", "nomx"])
        ]

if __name__ == "__main__":
    TestUtil.run(tests)
