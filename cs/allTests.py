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
    ("Slice/keyword", ["once", "nosilverlight"]),
    ("Slice/structure", ["once", "nosilverlight"]),
    ("Slice/macros", ["once", "nosilverlight"]),
    ("IceUtil/inputUtil", ["once", "nosilverlight"]),
    ("Ice/proxy", ["core"]),
    ("Ice/properties", ["once", "nosilverlight"]),
    ("Ice/operations", ["core"]),
    ("Ice/exceptions", ["core"]),
    ("Ice/ami", ["core"]),
    ("Ice/info", ["core", "noipv6", "nocompress", "nosocks"]),
    ("Ice/inheritance", ["core"]),
    ("Ice/facets", ["core"]),
    ("Ice/hold", ["core"]),
    ("Ice/objects", ["core"]),
    ("Ice/binding", ["core"]),
    ("Ice/faultTolerance", ["core"]),
    ("Ice/location", ["core"]),
    ("Ice/adapterDeactivation", ["core"]),
    ("Ice/slicing/exceptions", ["core"]),
    ("Ice/slicing/objects", ["core"]),
    ("Ice/checksum", ["core", "nocompact", "nosilverlight"]),
    ("Ice/dispatcher", ["core"]),
    ("Ice/stream", ["core"]),
    ("Ice/retry", ["core"]),
    ("Ice/timeout", ["core"]),
    ("Ice/servantLocator", ["core"]),
    ("Ice/interceptor", ["core", "nosilverlight"]),
    ("Ice/dictMapping", ["core"]),
    ("Ice/seqMapping", ["core"]),
    ("Ice/background", ["core", "nosilverlight"]),
    ("Ice/udp", ["core", "nosilverlight"]),
    ("Ice/defaultServant", ["core", "nosilverlight"]),
    ("Ice/defaultValue", ["core"]),
    ("Ice/threadPoolPriority", ["core", "nomono", "nosilverlight"]),
    ("Ice/invoke", ["core"]),
    ("Ice/hash", ["once", "nosilverlight"]),
    ("Ice/plugin", ["core", "nosilverlight"]),
    ("Ice/optional", ["core"]),
    ("Ice/admin", ["core", "nosilverlight"]),
    ("Ice/metrics", ["core", "nossl", "noipv6", "nocompress", "nosilverlight", "nosocks"]),
    ("Ice/enums", ["core"]),
    ("IceBox/admin", ["core", "noipv6", "nosilverlight", "nomx"]),
    ("IceBox/configuration", ["core", "noipv6", "nosilverlight", "nomx"]),
    ("Glacier2/router", ["service", "nosilverlight"]),
    ("Glacier2/sessionHelper", ["service", "nosilverlight", "nossl", "noipv6"]),
    ("IceGrid/simple", ["service", "nosilverlight"]),
    ("IceSSL/configuration", ["once", "novista", "nomono", "nocompact", "nosilverlight"]),
    ]

if __name__ == "__main__":
    TestUtil.run(tests)
