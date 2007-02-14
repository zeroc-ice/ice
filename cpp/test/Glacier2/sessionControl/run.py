#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

name = os.path.join("Glacier2", "sessionControl")
testdir = os.path.join(toplevel, "test", name)

server = os.path.join(testdir, "server")

print "starting server...",
serverPipe = os.popen(server + TestUtil.clientServerOptions + " 2>&1")
TestUtil.getServerPid(serverPipe)
TestUtil.getAdapterReady(serverPipe)
print "ok"

router = os.path.join(toplevel, "bin", "glacier2router")

command = router + TestUtil.clientServerOptions + \
          r' --Glacier2.Client.Endpoints="default -p 12347 -t 10000"' + \
          r' --Glacier2.Admin.Endpoints="tcp -h 127.0.0.1 -p 12348 -t 10000"' \
          r' --Glacier2.Server.Endpoints="default -p 12349 -t 10000"' + \
          r' --Glacier2.SessionManager="SessionManager:tcp -h 127.0.0.1 -p 12010 -t 10000"' \
          r' --Glacier2.PermissionsVerifier="Glacier2/NullPermissionsVerifier"'

print "starting router...",
starterPipe = os.popen(command + " 2>&1")
TestUtil.getServerPid(starterPipe)
TestUtil.getAdapterReady(starterPipe)
print "ok"

client = os.path.join(testdir, "client")

print "starting client...",
clientPipe = os.popen(client + TestUtil.clientServerOptions + " 2>&1")
TestUtil.getServerPid(clientPipe)
print "ok"

TestUtil.printOutputFromPipe(clientPipe)
clientStatus = TestUtil.closePipe(clientPipe)
if clientStatus:
    TestUtil.killServers()

if clientStatus or TestUtil.serverStatus():
    sys.exit(1)

sys.exit(0)
