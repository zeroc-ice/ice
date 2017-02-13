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

server = os.path.join(os.getcwd(), "server")
client = os.path.join(os.getcwd(), "client")
router = os.path.join(TestUtil.getGlacier2Router())

targets = []
if TestUtil.appverifier:
    targets = [server, client, router]
    TestUtil.setAppVerifierSettings(targets)


sys.stdout.write("starting server... ")
sys.stdout.flush()
serverProc = TestUtil.startServer(server, count=3)
print("ok")

args = r' --Glacier2.Client.Endpoints="default -p 12347"' + \
        r' --Ice.Admin.Endpoints="tcp -p 12348"' + \
        r' --Ice.Admin.InstanceName=Glacier2' + \
        r' --Glacier2.Server.Endpoints="default -p 12349"' + \
        r' --Glacier2.SessionManager="SessionManager:tcp -p 12010"' + \
        r' --Glacier2.PermissionsVerifier="Glacier2/NullPermissionsVerifier"' + \
        r' --Ice.Default.Locator="locator:default -p 12012"'

sys.stdout.write("starting router... ")
sys.stdout.flush()
starterProc = TestUtil.startServer(router, args, count=2) 
print("ok")

sys.stdout.write("starting client... ")
sys.stdout.flush()
proc = TestUtil.startClient(client, startReader = False)
print("ok")
proc.startReader()
proc.waitTestSuccess()

serverProc.waitTestSuccess()
starterProc.waitTestSuccess()

if TestUtil.appverifier:
    TestUtil.appVerifierAfterTestEnd(targets)
