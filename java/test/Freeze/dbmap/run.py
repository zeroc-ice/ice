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

dbdir = os.path.join(os.getcwd(), "db")
TestUtil.cleanDbDir(dbdir)

print "starting client...",
clientProc = TestUtil.startClient("test.Freeze.dbmap.Client", '"' + os.getcwd() + '"', startReader=False)
print "ok"
clientProc.startReader()
clientProc.waitTestSuccess()

