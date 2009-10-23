#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel))
from scripts import *

#
# List of all basic tests.
#
tests = [
    ("Slice/keyword", ["once"]),
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
    ("Ice/operations", ["core"]),
    ("Ice/slicing/exceptions", ["core"]),
    ("Ice/slicing/objects", ["core"]),
    ("Ice/custom", ["core"]),
    ("Ice/checksum", ["core"]),
    ("Ice/retry", ["core"]),
    ("Ice/timeout", ["core"]),
    ("Ice/servantLocator", ["core"]),
    ("Ice/blobject", ["core"]),
    ("Ice/defaultServant", ["core"])
    ]

if __name__ == "__main__":
    TestUtil.run(tests)
