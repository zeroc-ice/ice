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

server = os.path.join(os.getcwd(), "server")

print "starting server...",
serverProc = TestUtil.startServer(server)
print "ok"

router = TestUtil.getGlacier2Router()

args = ' --Ice.Warn.Dispatch=0' + \
       ' --Glacier2.AddSSLContext=1' + \
       ' --Glacier2.Client.Endpoints="tcp -h 127.0.0.1 -p 12347:ssl -h 127.0.0.1 -p 12348"' + \
       ' --Glacier2.Server.Endpoints="tcp -h 127.0.0.1"' \
       ' --Ice.Admin.Endpoints="tcp -h 127.0.0.1 -p 12349"' + \
       ' --Ice.Admin.InstanceName=Glacier2' + \
       ' --Glacier2.SessionManager="sessionmanager:tcp -h 127.0.0.1 -p 12350"' + \
       ' --Glacier2.PermissionsVerifier="Glacier2/NullPermissionsVerifier"' + \
       ' --Glacier2.SSLSessionManager="sslsessionmanager:tcp -h 127.0.0.1 -p 12350"' + \
       ' --Glacier2.SSLPermissionsVerifier="sslverifier:tcp -h 127.0.0.1 -p 12350"'

routerCfg = TestUtil.DriverConfig("server")
routerCfg.protocol = "ssl"
print "starting router...",
starterProc = TestUtil.startServer(router, args, routerCfg, count = 2)
print "ok"

clientCfg = TestUtil.DriverConfig("client")
clientCfg.protocol = "ssl"
client = os.path.join(os.getcwd(), "client")
print "starting client...",
clientProc = TestUtil.startClient(client, "", clientCfg, startReader = False)
print "ok"
clientProc.startReader()

clientProc.waitTestSuccess()
serverProc.waitTestSuccess()
starterProc.waitTestSuccess()

TestUtil.cleanup()
