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
# TODO: custom14 is commented out. We need to determine a java2 vs java5 build to determine
# which custom test to run.
#

#
# List of all basic tests.
#
tests = [
    ("Slice/generation", ["once"]),
    ("Slice/keyword", ["once"]),
    ("IceUtil/inputUtil", ["once"]),
    ("IceUtil/fileLock", ["once"]),
    ("Ice/proxy", ["core"]),
    ("Ice/operations", ["core"]),
    ("Ice/seqMapping", ["core"]),
    ("Ice/exceptions", ["core"]),
    ("Ice/ami", ["core"]),
    ("Ice/info", ["core", "noipv6", "nocompress"]),
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
    ("Ice/packagemd", ["core"]),
    ("Ice/stream", ["core"]),
    ("Ice/hold", ["core"]),
    ("Ice/retry", ["core"]),
    ("Ice/timeout", ["core"]),
    ("Ice/background", ["core"]),
    ("Ice/servantLocator", ["core"]),
    ("Ice/interceptor", ["core"]),
    ("Ice/udp", ["core"]),
    ("Ice/serialize", ["core"]),
    ("Ice/defaultServant", ["core"]),
    ("Ice/threadPoolPriority", ["core"]),
    ("Ice/classLoader", ["core"]),
    ("Ice/invoke", ["core"]),
    ("IceBox/configuration", ["core", "noipv6"]),
    ("Freeze/dbmap", ["once"]),
    ("Freeze/complex", ["once"]),
    ("Freeze/evictor", ["core"]),
    ("Freeze/fileLock", ["once"]),
    ("Glacier2/router", ["service"]),
    ("IceGrid/simple", ["service"]),
    ("IceSSL/configuration", ["once"])
    ]

if __name__ == "__main__":
    TestUtil.run(tests)
