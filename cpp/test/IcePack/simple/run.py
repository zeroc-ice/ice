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
# Add locator options for client and servers. All servers are now
# clients since they need to make requests to IcePack.
#
additionalOptions = " --Ice.Default.Locator=\"IcePack/locator:default -p 12346\" " + \
                    "--Ice.Adapter.TestAdapter.Endpoints=default"

#
# Start IcePack
# 
icePackPipe = IcePackAdmin.startIcePack(toplevel, "12346", testdir)

#
# Test client/server, collocated w/o automatic activation.
#
TestUtil.mixedClientServerTestWithOptions(toplevel, name, additionalOptions, additionalOptions)
TestUtil.collocatedTestWithOptions(toplevel, name, additionalOptions)

#
# Get adapter list, ensure that TestAdapter is in the list.
#
print "testing adapter registration...",
hasTestAdapter = 0;
icePackAdminPipe = IcePackAdmin.listAdapters(toplevel);
for adaptername in icePackAdminPipe.xreadlines():
    if adaptername.strip() == "TestAdapter":
        hasTestAdapter = 1
        
if hasTestAdapter == 0:
    print "failed!"
    TestUtil.killServers()
    sys.exit(1)

icePackStatus = icePackAdminPipe.close()
if icePackStatus:
    TestUtil.killServers()
    sys.exit(1)   
print "ok"

IcePackAdmin.removeAdapter(toplevel, "TestAdapter")

#
# This test doesn't work under Windows.
#
if TestUtil.isWin32() == 0 and TestUtil.protocol != "ssl":

    server = os.path.join(testdir, "server")
    client = os.path.join(testdir, "client")

    print "registering server with icepack...",
    IcePackAdmin.addServer(toplevel, "server", server, "", os.path.join(testdir, "simple_server.xml"));
    print "ok"
    
    print "testing adapter registration...",
    hasTestAdapter = 0;
    icePackAdminPipe = IcePackAdmin.listAdapters(toplevel);
    for adaptername in icePackAdminPipe.xreadlines():
        if adaptername.strip() == "TestAdapter":
            hasTestAdapter = 1
            
    if hasTestAdapter == 0:
        print "failed!"
        TestUtil.killServers()
        sys.exit(1)
        
    icePackStatus = icePackAdminPipe.close()
    if icePackStatus:
       TestUtil.killServers()
       sys.exit(1)
       
    print "ok"    

    updatedClientOptions = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel) + additionalOptions

    print "starting client...",
    clientPipe = os.popen(client + updatedClientOptions)
    print "ok"

    for output in clientPipe.xreadlines():
       print output,

    clientStatus = clientPipe.close()

    print "unregister server with icepack...",
    IcePackAdmin.removeServer(toplevel, "server");
    print "ok"

    if clientStatus:
	TestUtil.killServers()
	sys.exit(1)
    
IcePackAdmin.shutdownIcePack(toplevel, icePackPipe)

sys.exit(0)
