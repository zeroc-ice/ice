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
    ("Ice/proxy", ["core"]),
    ("Ice/operations", ["core"]),
    ("Ice/exceptions", ["core"]),
    ("Ice/info", ["core", "noipv6", "nocompress"]),
    ("Ice/inheritance", ["core"]),
    ("Ice/binding", ["core"]),
    ("Ice/checksum", ["core"]),
    ("Ice/facets", ["core"]),
    ("Ice/objects", ["core"]),
    ("Ice/slicing/exceptions", ["core"]),
    ("Ice/slicing/objects", ["core"]),
    ("Ice/defaultValue", ["core"]),
    ("Ice/optional", ["core"]),
    ("Ice/enums", ["core"]),
    ("Ice/ini", ["once"]),
    ("Ice/scope", ["once"]),
    ("Ice/acm", ["once"]),
    ("Slice/keyword", ["once"]),
    ("Slice/structure", ["once"]),
    ("Slice/macros", ["once"]),
    ]

if __name__ == "__main__":
    TestUtil.run(tests)
