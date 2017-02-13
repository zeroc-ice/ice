#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
    ("Slice/import", ["once"]),
    ("Slice/unicodePaths", ["once"]),
    ("Ice/adapterDeactivation", ["core"]),
    ("Ice/binding", ["core"]),
    ("Ice/exceptions", ["core"]),
    ("Ice/facets", ["core"]),
    ("Ice/faultTolerance", ["core"]),
    ("Ice/info", ["core", "noipv6", "nocompress"]),
    ("Ice/inheritance", ["core"]),
    ("Ice/location", ["core"]),
    ("Ice/objects", ["core"]),
    ("Ice/proxy", ["core"]),
    ("Ice/properties", ["once"]),
    ("Ice/operations", ["core"]),
    ("Ice/slicing/exceptions", ["core"]),
    ("Ice/slicing/objects", ["core"]),
    ("Ice/custom", ["core"]),
    ("Ice/checksum", ["core"]),
    ("Ice/timeout", ["core", "nocompress"]),
    ("Ice/servantLocator", ["core"]),
    ("Ice/blobject", ["core"]),
    ("Ice/defaultServant", ["core"]),
    ("Ice/defaultValue", ["core"]),
    ("Ice/ami", ["core", "nocompress"]),
    ("Ice/optional", ["core"]),
    ("Ice/enums", ["core"]),
    ("Ice/acm", ["core"])
    ]

if __name__ == "__main__":
    TestUtil.run(tests)
