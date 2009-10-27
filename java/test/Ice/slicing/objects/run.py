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

path = [ ".", "..", "../..", "../../..", "../../../..", "../../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise "can't find toplevel directory!"
sys.path.append(os.path.join(path[0]))
from scripts import *

testdir = os.getcwd()
testdirAMD = os.path.join(testdir, "..", "objectsAMD", "classes")

print "tests with regular server."
import copy
serverenv = copy.deepcopy(os.environ)
TestUtil.addClasspath(os.path.join(os.getcwd(), "sclasses"), serverenv)
TestUtil.clientServerTest(serverenv = serverenv)

print "tests with AMD server."
import copy
amdenv = copy.deepcopy(os.environ)
TestUtil.addClasspath(os.path.join(os.getcwd(), "..", "objectsAMD", "classes"), amdenv)
TestUtil.clientServerTest(serverenv = amdenv)

