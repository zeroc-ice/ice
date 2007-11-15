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
    toplevel = os.path.normpath(toplevel)
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
    "Ice/proxy",
    "Ice/operations",
    "Ice/exceptions",
    "Ice/inheritance",
    "Ice/binding",
    "Ice/facets",
    "Ice/objects",
    "Ice/slicing/exceptions",
    "Ice/slicing/objects",
    "Slice/keyword",
    ]

if __name__ == "__main__":
    TestUtil.run(tests)
