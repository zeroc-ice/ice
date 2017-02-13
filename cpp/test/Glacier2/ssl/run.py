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
router = TestUtil.getGlacier2Router()

targets = []
if TestUtil.appverifier:
    targets = [server, client, router]
    TestUtil.setAppVerifierSettings(targets)

sys.stdout.write("starting server... ")
sys.stdout.flush()
serverProc = TestUtil.startServer(server)
print("ok")

args = ' --Ice.Warn.Dispatch=0' + \
       ' --Glacier2.AddConnectionContext=1' + \
       ' --Glacier2.Client.Endpoints="tcp -h 127.0.0.1 -p 12347:ssl -h 127.0.0.1 -p 12348"' + \
       ' --Glacier2.Server.Endpoints="tcp -h 127.0.0.1"' \
       ' --Ice.Admin.Endpoints="tcp -h 127.0.0.1 -p 12349"' + \
       ' --Ice.Admin.InstanceName=Glacier2' + \
       ' --Glacier2.SessionManager="sessionmanager:tcp -h 127.0.0.1 -p 12350"' + \
       ' --Glacier2.PermissionsVerifier="verifier:tcp -h 127.0.0.1 -p 12350"' + \
       ' --Glacier2.SSLSessionManager="sslsessionmanager:tcp -h 127.0.0.1 -p 12350"' + \
       ' --Glacier2.SSLPermissionsVerifier="sslverifier:tcp -h 127.0.0.1 -p 12350"'

routerCfg = TestUtil.DriverConfig("server")
routerCfg.protocol = "ssl"
sys.stdout.write("starting router... ")
sys.stdout.flush()
starterProc = TestUtil.startServer(router, args, routerCfg, count = 2)
print("ok")

clientCfg = TestUtil.DriverConfig("client")
clientCfg.protocol = "ssl"

sys.stdout.write("starting client... ")
sys.stdout.flush()
clientProc = TestUtil.startClient(client, "", clientCfg, startReader = False)
print("ok")
clientProc.startReader()

clientProc.waitTestSuccess()
serverProc.waitTestSuccess()
starterProc.waitTestSuccess()

if TestUtil.appverifier:
    TestUtil.appVerifierAfterTestEnd(targets)
