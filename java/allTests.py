#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
    ("src/main/java/test/Slice/generation", ["once"]),
    ("src/main/java/test/Slice/keyword", ["once"]),
    ("src/main/java/test/Slice/macros", ["once"]),
    ("src/main/java/test/Slice/structure", ["once"]),
    ("src/main/java/test/IceUtil/inputUtil", ["once"]),
    ("src/main/java/test/IceUtil/fileLock", ["once"]),
    ("src/main/java/test/Ice/proxy", ["core"]),
    ("src/main/java/test/Ice/operations", ["core"]),
    ("src/main/java/test/Ice/seqMapping", ["core"]),
    ("src/main/java/test/Ice/exceptions", ["core"]),
    ("src/main/java/test/Ice/ami", ["core"]),
    ("src/main/java/test/Ice/info", ["core", "noipv6", "nocompress", "nosocks"]),
    ("src/main/java/test/Ice/inheritance", ["core"]),
    ("src/main/java/test/Ice/facets", ["core"]),
    ("src/main/java/test/Ice/objects", ["core"]),
    ("src/main/java/test/Ice/binding", ["core"]),
    ("src/main/java/test/Ice/faultTolerance", ["core"]),
    ("src/main/java/test/Ice/location", ["core"]),
    ("src/main/java/test/Ice/adapterDeactivation", ["core"]),
    ("src/main/java/test/Ice/slicing/exceptions", ["core"]),
    ("src/main/java/test/Ice/slicing/objects", ["core"]),
    ("src/main/java/test/Ice/custom", ["core"]),
    ("src/main/java/test/Ice/checksum", ["core"]),
    ("src/main/java/test/Ice/dispatcher", ["core"]),
    ("src/main/java/test/Ice/interrupt", ["core"]),
    ("src/main/java/test/Ice/packagemd", ["core"]),
    ("src/main/java/test/Ice/stream", ["core"]),
    ("src/main/java/test/Ice/hold", ["core"]),
    ("src/main/java/test/Ice/retry", ["core"]),
    ("src/main/java/test/Ice/timeout", ["core", "nocompress"]),
    ("src/main/java/test/Ice/acm", ["core"]),
    ("src/main/java/test/Ice/background", ["core", "nosocks"]),
    ("src/main/java/test/Ice/servantLocator", ["core"]),
    ("src/main/java/test/Ice/interceptor", ["core"]),
    ("src/main/java/test/Ice/udp", ["core"]),
    ("src/main/java/test/Ice/serialize", ["core"]),
    ("src/main/java/test/Ice/defaultServant", ["core"]),
    ("src/main/java/test/Ice/defaultValue", ["core"]),
    ("src/main/java/test/Ice/threadPoolPriority", ["core"]),
    ("src/main/java/test/Ice/classLoader", ["core"]),
    ("src/main/java/test/Ice/invoke", ["core"]),
    ("src/main/java/test/Ice/properties", ["once"]),
    ("src/main/java/test/Ice/plugin", ["core"]),
    ("src/main/java/test/Ice/hash", ["once"]),
    ("src/main/java/test/Ice/optional", ["once"]),
    ("src/main/java/test/Ice/admin", ["core"]),
    ("src/main/java/test/Ice/metrics", ["core", "nossl", "nows", "noipv6", "nocompress", "nosocks"]),
    ("src/main/java/test/Ice/enums", ["once"]),
    ("src/main/java/test/Ice/networkProxy", ["core", "noipv6", "nosocks"]),
    ("src/main/java/test/IceBox/admin", ["core", "noipv6", "nomx"]),
    ("src/main/java/test/IceBox/configuration", ["core", "noipv6", "nomx"]),
    ("src/main/java/test/Freeze/dbmap", ["once"]),
    ("src/main/java/test/Freeze/complex", ["once"]),
    ("src/main/java/test/Freeze/evictor", ["once"]),
    ("src/main/java/test/Freeze/fileLock", ["once"]),
    ("src/main/java/test/Glacier2/router", ["service"]),
    ("src/main/java/test/Glacier2/sessionHelper", ["service", "nossl", "nows", "noipv6"]),
    ("src/main/java/test/IceDiscovery/simple", ["service"]),
    ("src/main/java/test/IceGrid/simple", ["service"]),
    ("src/main/java/test/IceSSL/configuration", ["once"])
    ]

if __name__ == "__main__":
    TestUtil.run(tests)
