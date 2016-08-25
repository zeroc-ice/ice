#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
    ("IceUtil/sha1", ["once"]),
    ("IceUtil/stacktrace", ["once", "noc++11"]),
    ("Slice/errorDetection", ["once"]),
    ("Slice/keyword", ["once"]),
    ("Slice/structure", ["once"]),
    ("Slice/macros", ["once"]),
    ("Slice/headers", ["once", "nowin32"]),
    ("Ice/properties", ["once", "nomingw"]),
    ("Ice/proxy", ["core", "bt"]),
    ("Ice/operations", ["core", "bt"]),
    ("Ice/exceptions", ["core", "bt"]),
    ("Ice/ami", ["core", "nocompress", "bt"]),
    ("Ice/info", ["core", "noipv6", "nocompress", "nosocks"]),
    ("Ice/inheritance", ["core", "bt"]),
    ("Ice/facets", ["core", "bt"]),
    ("Ice/objects", ["core", "bt"]),
    ("Ice/optional", ["core", "bt"]),
    ("Ice/binding", ["core", "nosocks", "bt"]),
    ("Ice/faultTolerance", ["core", "novalgrind"]), # valgrind reports leak with aborted servers
    ("Ice/location", ["core"]),
    ("Ice/adapterDeactivation", ["core"]),
    ("Ice/slicing/exceptions", ["core", "bt"]),
    ("Ice/slicing/objects", ["core", "bt"]),
    ("Ice/gc", ["once", "noc++11"]),
    ("Ice/dispatcher", ["once"]),
    ("Ice/checksum", ["core"]),
    ("Ice/stream", ["core"]),
    ("Ice/hold", ["core", "bt"]),
    ("Ice/custom", ["core", "nossl", "nows"]),
    ("Ice/retry", ["core"]),
    ("Ice/timeout", ["core", "nocompress", "nosocks"]),
    ("Ice/acm", ["core", "bt"]),
    ("Ice/background", ["core", "nomingw", "nosocks"]),
    ("Ice/servantLocator", ["core", "bt"]),
    ("Ice/interceptor", ["core"]),
    ("Ice/library", ["once", "nostatic"]),
    ("Ice/stringConverter", ["core"]),
    ("Ice/udp", ["core"]),
    ("Ice/defaultServant", ["core"]),
    ("Ice/defaultValue", ["core"]),
    ("Ice/invoke", ["core"]),
    ("Ice/plugin", ["core", "nomingw"]),
    ("Ice/hash", ["once"]),
    ("Ice/admin", ["core", "noipv6"]),
    ("Ice/metrics", ["core", "nows", "noipv6", "nocompress", "nomingw", "nosocks"]),
    ("Ice/enums", ["once", "bt"]),
    ("Ice/logger", ["once"]),
    ("Ice/networkProxy", ["core", "noipv6", "nosocks"]),
    ("Ice/services", ["once", "nomingw"]),
    ("IceSSL/configuration", ["once", "novalgrind"]), # valgrind doesn't work well with openssl
    ("IceBox/configuration", ["core", "noipv6", "novc100", "nomingw", "nomx"]),
    ("IceBox/admin", ["core", "noipv6", "novc100", "nomingw", "nomx", "noc++11"]),
    ("IceStorm/single", ["service", "novc100", "noappverifier", "nomingw", "noc++11"]), # This test doesn't work with appverifier
    ("IceStorm/federation", ["service", "novc100", "nomingw", "noc++11"]),
    ("IceStorm/federation2", ["service", "novc100", "nomingw", "noc++11"]),
    ("IceStorm/stress", ["service", "stress", "novc100", "nomingw", "noc++11"]), # Too slow with appverifier.
    ("IceStorm/rep1", ["service", "novc100", "nomingw", "noc++11"]),
    ("IceStorm/repgrid", ["service", "novc100", "nomingw", "noc++11"]),
    ("IceStorm/repstress", ["service", "noipv6", "stress", "novc100", "nomingw", "noc++11"]),
    ("IceDiscovery/simple", ["service"]),
    ("IceGrid/simple", ["service", "novc100", "nomingw", "noc++11"]),
    ("IceGrid/fileLock", ["service", "novc100", "nomingw", "noc++11"]),
    ("IceGrid/deployer", ["service", "novc100", "nomingw", "noc++11"]),
    ("IceGrid/session", ["service", "novc100", "nomingw", "nows", "noc++11"]),
    ("IceGrid/update", ["service", "novc100", "nomingw", "noc++11"]),
    ("IceGrid/noRestartUpdate", ["service", "novc100", "nomingw", "noc++11"]),
    ("IceGrid/activation", ["service", "novc100", "nomingw", "noc++11"]),
    ("IceGrid/replicaGroup", ["service", "novc100", "nomingw", "noc++11", "nomultiarch"]),
    ("IceGrid/replication", ["service", "novc100", "nomingw", "nomx", "noc++11"]),
    ("IceGrid/allocation", ["service", "novc100", "nomingw", "noc++11"]),
    ("IceGrid/distribution", ["service", "novc100", "nomingw", "noc++11"]),
    ("IceGrid/admin", ["service", "novc100", "nomingw","nows", "noc++11"]),
    ("Glacier2/router", ["service", "novc100", "nomingw", "noc++11"]),
    ("Glacier2/attack", ["service", "novc100", "nomingw", "nomx", "noc++11"]),
    ("Glacier2/override", ["service", "novc100", "nomingw", "noc++11"]),
    ("Glacier2/sessionControl", ["service", "novc100", "nomingw", "noc++11"]),
    ("Glacier2/ssl", ["service", "novalgrind", "novc100", "nomingw", "noc++11"]), # valgrind doesn't work well with openssl
    ("Glacier2/dynamicFiltering", ["service", "novc100", "nomingw", "noc++11"]),
    ("Glacier2/staticFiltering", ["service", "noipv6", "novc100", "nomingw", "nomx", "noc++11"]),
    ("Glacier2/sessionHelper", ["core", "novc100", "nomingw", "noc++11"]),
    ("Glacier2/hashpassword", ["once", "nomingw", "novalgrind", "noappverifier", "noc++11"])
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
