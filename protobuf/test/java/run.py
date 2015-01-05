#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice Protobuf is licensed to you under the terms
# described in the ICE_PROTOBUF_LICENSE file included in this
# distribution.
#
# **********************************************************************

import os, sys

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

if "CLASSPATH" in os.environ:
    os.environ["CLASSPATH"] = os.environ["CLASSPATH"] + os.pathsep + "classes"
else:
    os.environ["CLASSPATH"] = "classes"

TestUtil.clientServerTest()
