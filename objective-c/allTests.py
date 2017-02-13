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
     ("Slice/keyword", []),
     ("Ice/proxy", ["core"]),
     ("Ice/ami", ["core", "nocompress"]),
     ("Ice/operations", ["core"]),
     ("Ice/exceptions", ["core"]),
     ("Ice/inheritance", ["core"]),
     ("Ice/invoke", ["core"]),
     ("Ice/metrics", ["core", "nows", "nossl", "noipv6", "nocompress"]),
     ("Ice/facets", ["core"]),
     ("Ice/objects", ["core"]),
     ("Ice/optional", ["core"]),
     ("Ice/interceptor", ["core"]),
     ("Ice/dispatcher", ["core"]),
     ("Ice/defaultServant", ["core"]),
     ("Ice/servantLocator", ["core"]),
     ("Ice/defaultValue", ["core"]),
     ("Ice/binding", ["core"]),
     ("Ice/stream", ["core"]),
     ("Ice/hold", ["core"]),
     ("Ice/faultTolerance", ["core"]),
     ("Ice/location", ["core"]),
     ("Ice/adapterDeactivation", ["core"]),
     ("Ice/slicing/exceptions", ["core"]),
     ("Ice/slicing/objects", ["core"]),
     ("Ice/retry", ["core"]),
     ("Ice/timeout", ["core", "nocompress"]),
     ("Ice/hash", ["core"]),
     ("Ice/info", ["core", "noipv6", "nocompress"]),
     ("Ice/enums", ["once"]),
     ("Ice/services", ["once"]),
     ("Ice/acm", ["core"]),
    ]

if __name__ == "__main__":
    TestUtil.run(tests)
