#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
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
testdir = os.path.join(toplevel, "test", "IcePack", "simple")

#
# Add locator options for the client and server. Since the server
# invokes on the locator it's also considered to be a client.
#
additionalOptions = " --Ice.Default.Locator=\"IcePack/Locator:default -p 12346\""

if os.path.exists(os.path.join(testdir, "db/registry")):
    TestUtil.cleanDbDir(os.path.join(testdir, "db/registry"))

#
# Start IcePack registry
# 
icePackRegistryPipe = IcePackAdmin.startIcePackRegistry(toplevel, "12346", testdir)

#
# Test client/server without on demand activation.
#
TestUtil.mixedClientServerTestWithOptions(toplevel, name, additionalOptions, additionalOptions)

#
# Shutdown the registry.
#
IcePackAdmin.shutdownIcePackRegistry(toplevel, icePackRegistryPipe)

TestUtil.cleanDbDir(os.path.join(testdir, "db/registry"))

if os.path.exists(os.path.join(testdir, "db/node/db")):
    TestUtil.cleanDbDir(os.path.join(testdir, "db/node/db"))

icePackRegistryPipe = IcePackAdmin.startIcePackRegistry(toplevel, "12346", testdir)
icePackNodePipe = IcePackAdmin.startIcePackNode(toplevel, testdir)

#
# Test client/server with on demand activation.
#
server = os.path.join(testdir, "server")
client = os.path.join(testdir, "client")

if TestUtil.protocol == "ssl":
    targets = "ssl"
else:
    targets = ""

print "registering server with icepack...",
IcePackAdmin.addServer(toplevel, "server", os.path.join(testdir, "simple_server.xml"), server, "", targets);
print "ok"
  
updatedClientOptions = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel) + additionalOptions

print "starting client...",
clientPipe = os.popen(client + updatedClientOptions)
print "ok"

for output in clientPipe.xreadlines():
    print output,
    
clientStatus = clientPipe.close()
if clientStatus:
    TestUtil.killServers()
    sys.exit(1)
    
print "unregister server with icepack...",
IcePackAdmin.removeServer(toplevel, "server");
print "ok"

IcePackAdmin.shutdownIcePackNode(toplevel, icePackNodePipe)
IcePackAdmin.shutdownIcePackRegistry(toplevel, icePackRegistryPipe)

sys.exit(0)
