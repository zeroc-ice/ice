#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
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
import IcePackAdmin

name = os.path.join("IcePack", "simple")
testdir = os.path.join(toplevel, "test", name)

#
# Add locator options for the client and server. Since the server
# invokes on the locator it's also considered to be a client.
#
additionalOptions = " --Ice.Default.Locator=\"IcePack/Locator:default -p 12346\""

IcePackAdmin.cleanDbDir(os.path.join(testdir, "db"))

#
# Start IcePack registry.
# 
icePackRegistryPipe = IcePackAdmin.startIcePackRegistry("12346", testdir)

#
# Test client/server without on demand activation.
#
additionalServerOptions=" --TestAdapter.Endpoints=default --TestAdapter.AdapterId=TestAdapter " + additionalOptions
TestUtil.mixedClientServerTestWithOptions(name, additionalServerOptions, additionalOptions)

#
# Shutdown the registry.
#
IcePackAdmin.shutdownIcePackRegistry(icePackRegistryPipe)

IcePackAdmin.cleanDbDir(os.path.join(testdir, "db"))

#
# Start IcePack registry and a node.
#
icePackRegistryPipe = IcePackAdmin.startIcePackRegistry("12346", testdir)
icePackNodePipe = IcePackAdmin.startIcePackNode(testdir)

#
# Test client/server with on demand activation.
#
server = os.path.join(testdir, "server")
client = os.path.join(testdir, "client")

print "registering server with icepack...",
IcePackAdmin.addServer("server", os.path.join(testdir, "simple_server.xml"), server, "", "");
print "ok"
  
print "starting client...",
clientPipe = os.popen(client + TestUtil.clientOptions + additionalOptions + " --with-deploy")
print "ok"

for output in clientPipe.xreadlines():
    print output,
    
clientStatus = clientPipe.close()
if clientStatus:
    TestUtil.killServers()
    sys.exit(1)
    
print "unregister server with icepack...",
IcePackAdmin.removeServer("server");
print "ok"

IcePackAdmin.shutdownIcePackNode(icePackNodePipe)
IcePackAdmin.shutdownIcePackRegistry(icePackRegistryPipe)

sys.exit(0)
