#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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

workqueue = os.path.join(os.getcwd(), "workqueue")

print "starting workqueue...",
client = TestUtil.spawnClient(workqueue)
print "ok"
client.waitTestSuccess()

match = os.path.join(os.getcwd(), "match")

print "starting signal match...",
client = TestUtil.spawnClient(match)
print "ok"
client.waitTestSuccess()

print "starting broadcast match...",
client = TestUtil.spawnClient(match + " -b")
print "ok"
client.waitTestSuccess()
