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
    ("Slice/keyword", ["once"]),
    ("Slice/structure", ["once"]),
    ("Slice/macros", ["once"]),
    ("IceUtil/inputUtil", ["once"]),
    ("Ice/proxy", ["core"]),
    ("Ice/properties", ["once"]),
    ("Ice/operations", ["core"]),
    ("Ice/exceptions", ["core"]),
    ("Ice/ami", ["core", "nocompress"]),
    ("Ice/info", ["core", "noipv6", "nocompress", "nosocks", "nomono"]),
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
    ("Ice/checksum", ["core", "nocompact"]),
    ("Ice/dispatcher", ["core"]),
    ("Ice/stream", ["core"]),
    ("Ice/retry", ["core"]),
    ("Ice/timeout", ["core", "nocompress"]),
    ("Ice/acm", ["core"]),
    ("Ice/servantLocator", ["core"]),
    ("Ice/interceptor", ["core"]),
    ("Ice/dictMapping", ["core"]),
    ("Ice/seqMapping", ["core"]),
    ("Ice/background", ["core", "nosocks"]),
    ("Ice/udp", ["core"]),
    ("Ice/defaultServant", ["core"]),
    ("Ice/defaultValue", ["core"]),
    ("Ice/threadPoolPriority", ["core", "nomono"]),
    ("Ice/invoke", ["core"]),
    ("Ice/hash", ["once"]),
    ("Ice/plugin", ["core"]),
    ("Ice/optional", ["core"]),
    ("Ice/admin", ["core"]),
    ("Ice/metrics", ["core", "nossl", "nows", "noipv6", "nocompress", "nosocks"]),
    ("Ice/enums", ["core"]),
    ("Ice/serialize", ["core"]),
    ("Ice/networkProxy", ["core", "noipv6", "nosocks"]),
    ("IceBox/admin", ["core", "noipv6", "nomx"]),
    ("IceBox/configuration", ["core", "noipv6", "nomx"]),
    ("Glacier2/router", ["service"]),
    ("Glacier2/sessionHelper", ["core"]),
    ("IceGrid/simple", ["service"]),
    ("IceSSL/configuration", ["once", "novista", "nomono", "nocompact"]),
    ]

if __name__ == "__main__":
    TestUtil.run(tests)
