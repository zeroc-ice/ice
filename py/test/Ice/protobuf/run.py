#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise "can't find toplevel directory!"
sys.path.append(os.path.join(path[0]))
from scripts import *

if os.system("protoc --python_out=. Test.proto") != 0:
    print "failed to compile Test.proto"
    sys.exit(1)

TestUtil.clientServerTest()

# Remove generated protobuf files.
if os.path.exists("Test_pb2.py"):
    os.unlink("Test_pb2.py")
if os.path.exists("Test_pb2.pyc"):
    os.unlink("Test_pb2.pyc")
