#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, re, getopt

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.abspath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

#
# List of all basic tests.
#
tests = [
    ("Slice/keyword", True),
    ("Ice/adapterDeactivation", False),
    ("Ice/binding", False),
    ("Ice/exceptions", False),
    ("Ice/facets", False),
    ("Ice/faultTolerance", False),
    ("Ice/inheritance", False),
    ("Ice/location", False),
    ("Ice/objects", False),
    ("Ice/proxy", False),
    ("Ice/operations", False),
    ("Ice/slicing/exceptions", False),
    ("Ice/slicing/objects", False),
    ("Ice/custom", False),
    ("Ice/checksum", False),
    ("Ice/retry", False),
    ("Ice/timeout", False),
    ("Ice/servantLocator", False),
    ("Ice/blobject", False)
    ]

if __name__ == "__main__":
    TestUtil.run(tests)
