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

import os, sys, time

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

name = os.path.join("Yellow", "basicYellow")
testdir = os.path.join(toplevel, "test", name)

IceBox = os.path.join(toplevel, "bin", "icebox")
IceBoxAdmin = os.path.join(toplevel, "bin", "iceboxadmin")

updatedServerOptions = TestUtil.serverOptions.replace("TOPLEVELDIR", toplevel)
updatedClientOptions = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel)
updatedClientServerOptions = TestUtil.clientServerOptions.replace("TOPLEVELDIR", toplevel)

IceBoxEndpoints=' --IceBox.ServiceManager.Endpoints="default -p 12345"'
IceBoxReference=' --IceBox.ServiceManager="ServiceManager:default -p 12345"'
YellowService=" --IceBox.Service.Yellow=YellowService:create" + \
		' --Yellow.Query.Endpoints="default -p 12346" --Yellow.Admin.Endpoints="default -p 12347"'

dbEnvName = os.path.join(testdir, "db")
TestUtil.cleanDbDir(dbEnvName)
YellowDBEnv=" --IceBox.DBEnvName.Yellow=" + dbEnvName

print "starting yellow service...",
command = IceBox + updatedClientServerOptions + IceBoxEndpoints + YellowService + YellowDBEnv
IceBoxPipe = os.popen(command)
TestUtil.getServerPid(IceBoxPipe)
TestUtil.getAdapterReady(IceBoxPipe)
print "ok"

client = os.path.join(testdir, "client")
#
# Start the client.
#
print "starting client...",
command = client + updatedClientOptions
clientPipe = os.popen(command)
print "ok"

for output in clientPipe.xreadlines():
    print output,
    
#
# Shutdown yellow.
#
print "shutting down yellow...",
command = IceBoxAdmin + updatedClientOptions + IceBoxReference + r' shutdown'
IceBoxAdminPipe = os.popen(command)
IceBoxAdminStatus = IceBoxAdminPipe.close()
if IceBoxAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
print "ok"

YellowStatus = IceBoxPipe.close()
clientStatus = clientPipe.close();

if YellowStatus or clientStatus:
    TestUtil.killServers()
    sys.exit(1)

sys.exit(0)
