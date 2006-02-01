#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

#
# Add locator options for the client and server. Since the server
# invokes on the locator it's also considered to be a client.
#
additionalOptions = " --Ice.Default.Locator=\"IceGrid/Locator:default -p 12345\""

IceGridAdmin.cleanDbDir(os.path.join(testdir, "db"))

#
# Start IceGrid registry.
# 
iceGridRegistryThread = IceGridAdmin.startIceGridRegistry("12345", testdir, 1)

#
# Test client/server without on demand activation.
#
additionalServerOptions=" --TestAdapter.Endpoints=default --TestAdapter.AdapterId=TestAdapter " + additionalOptions
TestUtil.mixedClientServerTestWithOptions(name, additionalServerOptions, additionalOptions)

#
# Shutdown the registry.
#
IceGridAdmin.shutdownIceGridRegistry()
iceGridRegistryThread.join()

IceGridAdmin.cleanDbDir(os.path.join(testdir, "db"))

#
# Start IceGrid registry and a node.
#
iceGridRegistryThread = IceGridAdmin.startIceGridRegistry("12345", testdir, 0)
iceGridNodeThread = IceGridAdmin.startIceGridNode(testdir)

#
# Test client/server with on demand activation.
#
server = os.path.join(testdir, "server")
client = os.path.join(testdir, "client")

print "registering server with icegrid...",
IceGridAdmin.addApplication(os.path.join(testdir, "simple_server.xml"), "test.dir=" + testdir)
print "ok"
  
print "starting client...",
clientPipe = os.popen(client + TestUtil.clientOptions + additionalOptions + " --with-deploy" + " 2>&1")
print "ok"

TestUtil.printOutputFromPipe(clientPipe)
    
clientStatus = TestUtil.closePipe(clientPipe)
    
print "unregister server with icegrid...",
IceGridAdmin.removeApplication("Test")
print "ok"

IceGridAdmin.shutdownIceGridNode()
iceGridNodeThread.join()
IceGridAdmin.shutdownIceGridRegistry()
iceGridRegistryThread.join()

if clientStatus:
    sys.exit(1)
else:
    sys.exit(0)
