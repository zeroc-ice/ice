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

name = os.path.join("IceGrid", "session")
testdir = os.path.join(toplevel, "test", name)
client = os.path.join(testdir, "client")

#
# Add locator options for the client and server. Since the server
# invokes on the locator it's also considered to be a client.
#
additionalOptions = " --Ice.Default.Locator=\"IceGrid/Locator:default -p 12010\" " + \
    "--Ice.PrintAdapterReady=0 --Ice.PrintProcessId=0 --IceDir=\"" + toplevel + "\" --TestDir=\"" + testdir + "\""

IceGridAdmin.cleanDbDir(os.path.join(testdir, "db"))
iceGridRegistryThread = IceGridAdmin.startIceGridRegistry("12010", testdir, 1)
iceGridNodeThread = IceGridAdmin.startIceGridNode(testdir)

node1Dir = os.path.join(testdir, "db", "node-1")
os.mkdir(node1Dir)
    
print "starting client...",
clientPipe = os.popen(client + TestUtil.clientServerOptions + additionalOptions + " 2>&1")
print "ok"

try:
    TestUtil.printOutputFromPipe(clientPipe)
except:
    pass
    
clientStatus = TestUtil.closePipe(clientPipe)

IceGridAdmin.shutdownIceGridNode()
iceGridNodeThread.join()
IceGridAdmin.shutdownIceGridRegistry()
iceGridRegistryThread.join()

if clientStatus:
    sys.exit(1)
else:
    sys.exit(0)
