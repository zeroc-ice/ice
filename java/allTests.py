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
    "IceUtil/inputUtil",
    "Ice/proxy",
    "Ice/operations",
    "Ice/exceptions",
    "Ice/inheritance",
    "Ice/facets",
    "Ice/objects",
    "Ice/binding",
    "Ice/faultTolerance",
    "Ice/location",
    "Ice/adapterDeactivation",
    "Ice/slicing/exceptions",
    "Ice/slicing/objects",
    "Ice/custom",
    "Ice/checksum",
    "Ice/package",
    "Ice/stream",
    "Ice/hold",
    "Ice/retry",
    "Ice/timeout",
    "Ice/background",
    "Ice/servantLocator",
    "Ice/threads",
    "Ice/interceptor",
    "Freeze/dbmap",
    "Freeze/complex",
    "Freeze/evictor",
    "Freeze/oldevictor",
    "Glacier2/router",
    "Glacier2/attack",
    "IceGrid/simple",
    "IceSSL/configuration",
    ]

if __name__ == "__main__":
    TestUtil.run(tests)
