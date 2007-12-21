#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    "Slice/keyword",
#    "Ice/adapterDeactivation",
    "Ice/binding",
    "Ice/exceptions",
    "Ice/facets",
    "Ice/faultTolerance",
    "Ice/inheritance",
    "Ice/location",
    "Ice/objects",
    "Ice/proxy",
    "Ice/operations",
    "Ice/slicing/exceptions",
    "Ice/slicing/objects",
    "Ice/custom",
    "Ice/checksum",
    "Ice/retry",
    "Ice/timeout",
    "Ice/servantLocator",
    "Ice/blobject",
    ]

if __name__ == "__main__":
    TestUtil.run(tests)
