#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
# Mutable Realms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
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

TestUtil.cleanDbDir(os.path.join(testdir, "db/db"))

#
# Start IcePack
# 
icePackPipe = IcePackAdmin.startIcePack(toplevel, "12346", testdir)

#
# Test client/server, collocated without on demand activation.
#
TestUtil.mixedClientServerTestWithOptions(toplevel, name, additionalOptions, additionalOptions)
TestUtil.collocatedTestWithOptions(toplevel, name, additionalOptions)

#
# Remove the adapter (registered by the server) before deploying the
# server.
#
IcePackAdmin.removeAdapter(toplevel, "TestAdapter")

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

IcePackAdmin.shutdownIcePack(toplevel, icePackPipe)

sys.exit(0)
