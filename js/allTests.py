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
    ("Ice/acm", ["core"]),
    ("Ice/ami", ["core"]),
    ("Ice/binding", ["core"]),
    ("Ice/defaultValue", ["once"]),
    ("Ice/enums", ["once"]),
    ("Ice/exceptions", ["core"]),
    ("Ice/exceptionsBidir", ["once"]),
    ("Ice/facets", ["core"]),
    ("Ice/facetsBidir", ["core"]),
    ("Ice/hashmap", ["once"]),
    ("Ice/hold", ["core"]),
    ("Ice/info", ["core"]),
    ("Ice/inheritance", ["once"]),
    ("Ice/inheritanceBidir", ["once"]),
    ("Ice/location", ["once"]),
    ("Ice/objects", ["core"]),
    ("Ice/operations", ["core"]),
    ("Ice/operationsBidir", ["once"]),
    ("Ice/optional", ["core"]),
    ("Ice/optionalBidir", ["once"]),
    ("Ice/promise", ["once"]),
    ("Ice/properties", ["once"]),
    ("Ice/proxy", ["core"]),
    ("Ice/retry", ["core"]),
    ("Ice/slicing/exceptions", ["core"]),
    ("Ice/slicing/objects", ["core"]),
    ("Ice/timeout", ["core"]),
    ("Ice/number", ["once"]),
    ("Glacier2/router", ["service"]),
    ]

if __name__ == "__main__":
    TestUtil.run(tests)
