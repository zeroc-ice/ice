#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
    ("Slice/generation", ["once"]),
    ("Slice/keyword", ["once"]),
    ("Slice/macros", ["once"]),
    ("Slice/structure", ["once"]),
    ("IceUtil/inputUtil", ["once"]),
    ("IceUtil/fileLock", ["once"]),
    ("Ice/proxy", ["core"]),
    ("Ice/operations", ["core"]),
    ("Ice/seqMapping", ["core"]),
    ("Ice/exceptions", ["core"]),
    ("Ice/ami", ["core", "nocompress"]),
    ("Ice/info", ["core", "noipv6", "nocompress", "nosocks"]),
    ("Ice/inheritance", ["core"]),
    ("Ice/facets", ["core"]),
    ("Ice/objects", ["core"]),
    ("Ice/binding", ["core"]),
    ("Ice/faultTolerance", ["core"]),
    ("Ice/location", ["core"]),
    ("Ice/adapterDeactivation", ["core"]),
    ("Ice/slicing/exceptions", ["core"]),
    ("Ice/slicing/objects", ["core"]),
    ("Ice/custom", ["core"]),
    ("Ice/checksum", ["core"]),
    ("Ice/dispatcher", ["core"]),
    ("Ice/interrupt", ["core", "nocompress"]),
    ("Ice/packagemd", ["core"]),
    ("Ice/stream", ["core"]),
    ("Ice/hold", ["core"]),
    ("Ice/retry", ["core"]),
    ("Ice/timeout", ["core", "nocompress"]),
    ("Ice/acm", ["core"]),
    ("Ice/background", ["core", "nosocks"]),
    ("Ice/servantLocator", ["core"]),
    ("Ice/interceptor", ["core"]),
    ("Ice/udp", ["core"]),
    ("Ice/serialize", ["core"]),
    ("Ice/defaultServant", ["core"]),
    ("Ice/defaultValue", ["core"]),
    ("Ice/threadPoolPriority", ["core"]),
    ("Ice/classLoader", ["core"]),
    ("Ice/invoke", ["core"]),
    ("Ice/properties", ["once"]),
    ("Ice/plugin", ["core"]),
    ("Ice/hash", ["once"]),
    ("Ice/optional", ["once"]),
    ("Ice/admin", ["core"]),
    ("Ice/metrics", ["core", "nossl", "nows", "noipv6", "nocompress", "nosocks"]),
    ("Ice/enums", ["once"]),
    ("Ice/networkProxy", ["core", "noipv6", "nosocks"]),
    ("IceBox/admin", ["core", "noipv6", "nomx"]),
    ("IceBox/configuration", ["core", "noipv6", "nomx"]),
    ("Freeze/dbmap", ["once"]),
    ("Freeze/complex", ["once"]),
    ("Freeze/evictor", ["once"]),
    ("Freeze/fileLock", ["once"]),
    ("Glacier2/router", ["service"]),
    ("Glacier2/sessionHelper", ["core"]),
    ("IceDiscovery/simple", ["service"]),
    ("IceGrid/simple", ["service"]),
    ("IceSSL/configuration", ["once"])
    ]

if __name__ == "__main__":
    TestUtil.run(tests)
