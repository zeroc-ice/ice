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

if not os.environ.has_key('ICE_HOME'):
    print "ICE_HOME is not defined."
    sys.exit(0)

ice_home = os.environ['ICE_HOME']

testdir = os.path.join(toplevel, "test", "IcePack", "simple")
name = os.path.join("IcePack", "simple")

#
# Add locator options for client and servers. All servers are now
# clients since they need to make requests to IcePack.
#
additionalOptions = " --Ice.Default.Locator=\"IcePack/locator:default -p 12346\""

#
# Start IcePack
# 
icePackPipe = IcePackAdmin.startIcePack(ice_home, "12346", testdir)

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
icePackAdminPipe = IcePackAdmin.listAdapters(ice_home);
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

IcePackAdmin.removeAdapter(ice_home, "TestAdapter")

#
# This test doesn't work under Windows.
#
if TestUtil.isWin32() == 0:

    classpath = os.path.join(toplevel, "lib") + TestUtil.sep + os.path.join(testdir, "classes") + TestUtil.sep + \
	os.getenv("CLASSPATH", "")
    client = "java -ea -classpath \"" + classpath + "\" Client "

    print "registering server with icepack...",
    IcePackAdmin.addServer(ice_home, "server", "", classpath, os.path.join(testdir, "simple_server.xml"));
    print "ok"

    print "testing adapter registration...",
    hasTestAdapter = 0;
    icePackAdminPipe = IcePackAdmin.listAdapters(ice_home);
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

#    IcePackAdmin.startServer(toplevel, "server")

    updatedClientOptions = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel)

    print "starting client...",
    clientPipe = os.popen(client + updatedClientOptions + additionalOptions)
    print "ok"

    for output in clientPipe.xreadlines():
        print output,

    clientStatus = clientPipe.close()
    if clientStatus:
	TestUtil.killServers()
	sys.exit(1)

    print "unregister server with icepack...",
    IcePackAdmin.removeServer(ice_home, "server");
    print "ok"
    
IcePackAdmin.shutdownIcePack(ice_home, icePackPipe)

sys.exit(0)
