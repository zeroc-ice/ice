#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, time

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
router = TestUtil.getGlacier2Router()
client = os.path.join(os.getcwd(), "client")

targets = []
if TestUtil.appverifier:
    targets = [server, client, router]
    TestUtil.setAppVerifierSettings(targets)

print "starting server...",
serverProc = TestUtil.startServer(server)
print "ok"

args =    ' --Glacier2.Client.Endpoints="default -p 12347"' + \
          ' --Ice.Admin.Endpoints="tcp -p 12348"' + \
          ' --Ice.Admin.InstanceName=Glacier2' + \
          ' --Glacier2.Server.Endpoints="default -p 12349"' + \
          ' --Glacier2.SessionManager="SessionManager:tcp -p 12010"' \
          ' --Glacier2.PermissionsVerifier="Glacier2/NullPermissionsVerifier"'

print "starting router...",
starterProc = TestUtil.startServer(router, args, count = 2)
print "ok"



#
# The test may sporadically fail without this slight pause.
#
time.sleep(1)

print "starting client...",
clientProc = TestUtil.startClient(client, startReader = False)
print "ok"
clientProc.startReader()

clientProc.waitTestSuccess()
serverProc.waitTestSuccess()
starterProc.waitTestSuccess()

if TestUtil.appverifier:
    TestUtil.appVerifierAfterTestEnd(targets)

