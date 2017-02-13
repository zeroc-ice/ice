#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

path = [ ".", "..", "../..", "../../..", "../../../..", "../../../../..", "../../../../../..",
         "../../../../../../..", "../../../../../../../..", "../../../../../../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

TestUtil.addClasspath(os.path.join(os.getcwd(), "classes"))

num = 5

serverProc = []
for i in range(0, num):
    sys.stdout.write("starting server #%d... " % (i + 1))
    sys.stdout.flush()
    serverProc.append(TestUtil.startServer("test.Ice.udp.Server", "%d" % i , adapter="McastTestAdapter"))
    print("ok")

sys.stdout.write("starting client... ")
sys.stdout.flush()
clientProc = TestUtil.startClient("test.Ice.udp.Client", "%d" % num, startReader=False)
print("ok")
clientProc.startReader()

clientProc.waitTestSuccess()
for p in serverProc:
    p.waitTestSuccess()
