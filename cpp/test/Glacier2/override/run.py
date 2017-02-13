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

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil

router = TestUtil.getGlacier2Router()

if TestUtil.appverifier:
    TestUtil.setAppVerifierSettings([router])

def startRouter():
    #
    # Note: we limit the send buffer size with Ice.TCP.SndSize, the
    # test relies on send() blocking
    #
    args = ' --Ice.Warn.Dispatch=0' + \
           ' --Ice.Warn.Connections=0' + \
           ' --Ice.TCP.SndSize=100000' + \
           ' --Ice.ThreadPool.Server.Serialize=1' + \
           ' --Ice.ThreadPool.Client.Serialize=1' + \
           ' --Glacier2.Filter.Category.Accept="c"' + \
           ' --Glacier2.SessionTimeout="30"' + \
           ' --Glacier2.Client.Endpoints="default -p 12347"' + \
           ' --Glacier2.Server.Endpoints="tcp -h 127.0.0.1"' \
           ' --Ice.Admin.Endpoints="tcp -h 127.0.0.1 -p 12348"' + \
           ' --Glacier2.PermissionsVerifier=Glacier2/NullPermissionsVerifier' + \
           ' --Glacier2.Client.ForwardContext=1' + \
           ' --Glacier2.Client.Trace.Override=0' + \
           ' --Glacier2.Client.Trace.Request=0' + \
           ' --Glacier2.Server.Trace.Override=0' + \
           ' --Glacier2.Server.Trace.Request=0' + \
           ' --Ice.Admin.InstanceName="Glacier2"' + \
           ' --Glacier2.Client.Buffered=1 --Glacier2.Server.Buffered=1' + \
           ' --Glacier2.Client.SleepTime=50 --Glacier2.Server.SleepTime=50'

    sys.stdout.write("starting router in buffered mode... ")
    sys.stdout.flush()
    starterProc = TestUtil.startServer(router, args, count=2)
    print("ok")
    return starterProc

name = os.path.join("Glacier2", "override")

starterProc = startRouter()
TestUtil.clientServerTest(name, additionalClientOptions = " --shutdown")
starterProc.waitTestSuccess()

if TestUtil.appverifier:
    TestUtil.appVerifierAfterTestEnd([router])
