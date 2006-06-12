#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
import IceGridAdmin

name = os.path.join("IceGrid", "simple")
testdir = os.path.join(toplevel, "test", name)
server = os.path.join(testdir, "server")
client = os.path.join(testdir, "client")

additionalOptions = " --Ice.Default.Locator=\"IceGrid/Locator:default -p 12010\""
additionalServerOptions=" --TestAdapter.Endpoints=default --TestAdapter.AdapterId=TestAdapter " + additionalOptions

iceGridRegistryPipe = IceGridAdmin.startIceGridRegistry(testdir, 1)

#
# Test client/server without on demand activation.
#
print "starting sever...",
serverPipe = os.popen(server + TestUtil.serverOptions + additionalServerOptions + " 2>&1")
TestUtil.getServerPid(serverPipe)
TestUtil.getAdapterReady(serverPipe)
print "ok"
    
print "starting client...",
clientPipe = os.popen(client + TestUtil.clientOptions + additionalOptions + " 2>&1")
print "ok"

TestUtil.printOutputFromPipe(clientPipe)

clientStatus = TestUtil.closePipe(clientPipe)
if clientStatus:
    killServers()    

IceGridAdmin.shutdownIceGridRegistry()
TestUtil.joinServers()

if clientStatus or TestUtil.serverStatus():
    sys.exit(1)

#
# Test client/server with on demand activation.
#
IceGridAdmin.iceGridTest(name, "simple_server.xml", "--with-deploy")
sys.exit(0)
