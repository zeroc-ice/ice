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

#
# Add locator options for client and servers. All servers are now
# clients since they need to make requests to IcePack.
#
additionalOptions = " --Ice.Default.Locator=\"IcePack/locator:default -p 12346\""

#
# Start IcePack
# 
icePackPipe = IcePackAdmin.startIcePack(toplevel, "12346")

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
    if adaptername == "TestAdapter\n":
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
if TestUtil.isWin32() == 0:

    testdir = os.path.join(toplevel, "test", "IcePack", "simple")
    server = os.path.join(testdir, "server")
    client = os.path.join(testdir, "client")

    #
    # Don't pass Ice.Locator.* properties for the server. The IcePack
    # activator should take care of this.
    #
    updatedClientServerOptions = TestUtil.clientServerOptions.replace("TOPLEVELDIR", toplevel)

    print "registering server with icepack...",
    IcePackAdmin.addServer(toplevel, "server", server, "", updatedClientServerOptions, "TestAdapter")
    print "ok"
    
    print "testing adapter registration...",
    hasTestAdapter = 0;
    icePackAdminPipe = IcePackAdmin.listAdapters(toplevel);
    for adaptername in icePackAdminPipe.xreadlines():
        if adaptername == "TestAdapter\n":
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

#    IcePackAdmin.startServer(toplevel, "server")

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
    
IcePackAdmin.shutdownIcePack(toplevel, icePackPipe)

sys.exit(0)
