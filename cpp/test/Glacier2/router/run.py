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

def startRouter(buffered):

    args = ' --Ice.Warn.Dispatch=0' + \
           ' --Ice.Warn.Connections=0' + \
           ' --Glacier2.Filter.Category.Accept="c1 c2"' + \
           ' --Glacier2.Filter.Category.AcceptUser="2"' + \
           ' --Glacier2.SessionTimeout="30"' + \
           ' --Glacier2.Client.Endpoints="default -p 12347"' + \
           ' --Glacier2.Server.Endpoints="tcp -h 127.0.0.1"' \
           ' --Ice.Admin.Endpoints="tcp -h 127.0.0.1 -p 12348"' + \
           ' --Ice.Admin.InstanceName="Glacier2"' + \
           ' --Glacier2.CryptPasswords="%s"' % os.path.join(os.getcwd(), "passwords")

    if buffered:
        args += ' --Glacier2.Client.Buffered=1 --Glacier2.Server.Buffered=1' 
        sys.stdout.write("starting router in buffered mode... ")
        sys.stdout.flush()
    else:
        args += ' --Glacier2.Client.Buffered=0 --Glacier2.Server.Buffered=0'
        sys.stdout.write("starting router in unbuffered mode... ")
        sys.stdout.flush()

    starterProc = TestUtil.startServer(router, args, count=2)

    print("ok")
    return starterProc

name = os.path.join("Glacier2", "router")

#
# Generate the crypt passwords file
#
TestUtil.hashPasswords(os.path.join(os.getcwd(), "passwords"),
                        {"userid": "abc123", "userid-0": "abc123", "userid-1": "abc123",
                         "userid-2": "abc123", "userid-3": "abc123","userid-4": "abc123"})
#
# We first run the test with unbuffered mode.
#
starterProc = startRouter(False)
TestUtil.clientServerTest(name, additionalClientOptions = " --shutdown")
starterProc.waitTestSuccess()

#
# Then we run the test in buffered mode.
# 
starterProc = startRouter(True)
TestUtil.clientServerTest()

#
# We run the test again, to check whether the glacier router can
# handle multiple clients. Also, when we run for the second time, we
# want the client to shutdown the router after running the tests.
#
TestUtil.clientServerTest(name, additionalClientOptions = " --shutdown")

starterProc.waitTestSuccess()

if TestUtil.appverifier:
    TestUtil.appVerifierAfterTestEnd([router])
