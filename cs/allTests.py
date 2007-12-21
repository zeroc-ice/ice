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
tests = [ \
    "Slice/keyword", \
    "IceUtil/inputUtil", \
    "Ice/proxy", \
    "Ice/operations", \
    "Ice/exceptions", \
    "Ice/inheritance", \
    "Ice/facets", \
    "Ice/hold", \
    "Ice/objects", \
    "Ice/binding", \
    "Ice/faultTolerance", \
    "Ice/location", \
    "Ice/adapterDeactivation", \
    "Ice/slicing/exceptions", \
    "Ice/slicing/objects", \
    "Ice/checksum", \
    "Ice/stream", \
    "Ice/retry", \
    "Ice/timeout", \
    "Ice/servantLocator", \
    "Ice/interceptor", \
    "Ice/dictMapping", \
    "Ice/seqMapping", \
    "Ice/background", \
    "Ice/threads", \
    "IceBox/configuration", \
    "Glacier2/router", \
    "Glacier2/attack", \
    "IceGrid/simple" \
    ]

#if not TestUtil.mono:
#    tests += ["IceSSL/configuration"]

if __name__ == "__main__":
    TestUtil.run(tests)
