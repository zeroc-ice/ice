#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
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

if not os.environ.has_key('ICE_HOME'):
    print "ICE_HOME is not defined."
    sys.exit(0)

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil
import IcePackAdmin

name = os.path.join("IcePack", "simple")
testdir = os.path.join(toplevel, "test", name)
os.environ["CLASSPATH"] = os.path.join(testdir, "classes") + TestUtil.sep + os.getenv("CLASSPATH", "")

#
# Add locator options for the client and server. Since the server
# invokes on the locator it's also considered to be a client.
#
additionalOptions = " --Ice.Default.Locator=\"IcePack/Locator:default -p 12345\""

IcePackAdmin.cleanDbDir(os.path.join(testdir, "db"))

#
# Start IcePack registry.
# 
icePackRegistryThread = IcePackAdmin.startIcePackRegistry("12345", testdir)

#
# Test client/server without on demand activation.
#
additionalServerOptions=" --TestAdapter.Endpoints=\"default -t 30000\" --TestAdapter.AdapterId=TestAdapter " + additionalOptions
TestUtil.mixedClientServerTestWithOptions(additionalServerOptions, additionalOptions)

#
# Shutdown the registry.
#
IcePackAdmin.shutdownIcePackRegistry()
icePackRegistryThread.join()

IcePackAdmin.cleanDbDir(os.path.join(testdir, "db"))

#
# Start IcePack registry and a node.
#
icePackRegistryThread = IcePackAdmin.startIcePackRegistry("12345", testdir)
icePackNodeThread = IcePackAdmin.startIcePackNode(testdir)

#
# Test client/server with on demand activation.
#
client = "java -ea Client"

print "registering server with icepack...",
IcePackAdmin.addServer(os.path.join(testdir, "simple_server.xml"), "test.dir=" + testdir);
print "ok"

print "starting client...",
clientPipe = os.popen(client + TestUtil.clientOptions + additionalOptions + " --with-deploy" + " 2>&1")
print "ok"

TestUtil.printOutputFromPipe(clientPipe)
    
clientStatus = clientPipe.close()
if clientStatus:
    TestUtil.killServers()
    sys.exit(1)

print "unregister server with icepack...",
IcePackAdmin.removeServer("server");
print "ok"
    
IcePackAdmin.shutdownIcePackNode()
icePackNodeThread.join()
IcePackAdmin.shutdownIcePackRegistry()
icePackRegistryThread.join()

sys.exit(0)
