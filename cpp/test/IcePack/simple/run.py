#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
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

icePack = os.path.join(toplevel, "bin", "icepack")
icePackAdmin = os.path.join(toplevel, "bin", "icepackadmin")

updatedServerOptions = TestUtil.serverOptions.replace("TOPLEVELDIR", toplevel)
updatedClientOptions = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel)
updatedClientServerOptions = TestUtil.clientServerOptions.replace("TOPLEVELDIR", toplevel)

print "starting icepack...",
command = icePack + updatedClientServerOptions + ' --nowarn' + \
          r' "--IcePack.Forward.Endpoints=default -p 12346 -t 5000"' + \
          r' "--IcePack.Admin.Endpoints=default -p 12347 -t 5000"'
icePackPipe = os.popen(command)
TestUtil.getServerPid(icePackPipe)
TestUtil.getAdapterReady(icePackPipe)
TestUtil.getAdapterReady(icePackPipe)
print "ok"

print "registering server with icepack...",
command = icePackAdmin + updatedClientOptions + \
          r' "--IcePack.Admin.Endpoints=default -p 12347 -t 5000" -e "add \"test:default -p 12345 -t 5000\""'
icePackAdminPipe = os.popen(command)
icePackAdminStatus = icePackAdminPipe.close()
if icePackAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
print "ok"

name = os.path.join("IcePack", "simple")
TestUtil.clientServerTest(toplevel, name)
TestUtil.collocatedTest(toplevel, name)

#
# This test doesn't work under Windows.
#
if sys.platform != "cygwin" and sys.platform != "win32":

    testdir = os.path.join(toplevel, "test", "IcePack", "simple")
    server = os.path.join(testdir, "server")
    client = os.path.join(testdir, "client")

    print "registering server with icepack for automatic activation...",
    command = icePackAdmin + updatedClientOptions + \
              r' "--IcePack.Admin.Endpoints=default -p 12347 -t 5000"' + \
              r' -e "add \"test:default -p 12345 -t 5000\" ' + server + updatedServerOptions + '"'
    icePackAdminPipe = os.popen(command)
    icePackAdminStatus = icePackAdminPipe.close()
    if icePackAdminStatus:
        TestUtil.killServers()
        sys.exit(1)
    print "ok"

    print "starting client...",
    exit
    clientPipe = os.popen(client + updatedClientOptions)
    print "ok"

    for output in clientPipe.xreadlines():
        print output,

    clientStatus = clientPipe.close()

    if clientStatus:
	killServers()
	sys.exit(1)
    
print "shutting down icepack...",
command = icePackAdmin + updatedClientOptions + \
          r' "--IcePack.Admin.Endpoints=default -p 12347 -t 5000" -e "shutdown"'
icePackAdminPipe = os.popen(command)
icePackAdminStatus = icePackAdminPipe.close()
if icePackAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
print "ok"

icePackStatus = icePackPipe.close()

if icePackStatus:
    TestUtil.killServers()
    sys.exit(1)

sys.exit(0)
