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

command = icePack + updatedClientServerOptions + ' --nowarn' + \
       r' "--Ice.Adapter.Forward.Endpoints=default -p 12346 -t 5000"' + \
       r' "--Ice.Adapter.Admin.Endpoints=default -p 12347 -t 5000"'
print "starting icepack...",
icePackPipe = os.popen(command)
TestUtil.getServerPid(icePackPipe)
TestUtil.getAdapterReady(icePackPipe)
TestUtil.getAdapterReady(icePackPipe)
print "ok"

secure = ""
if TestUtil.protocol == "ssl":
    secure = " -s "

command = icePackAdmin + updatedClientOptions + \
        r' "--Ice.Adapter.Admin.Endpoints=' + TestUtil.protocol + ' -p 12347 -t 5000"' + \
        r' -e "add \"test' + secure + ':' + TestUtil.protocol + r' -p 12345 -t 5000\""'
print "registering server with icepack...",
icePackAdminPipe = os.popen(command)
icePackAdminPipe.close()
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

    command = icePackAdmin + updatedClientOptions + \
            r' "--Ice.Adapter.Admin.Endpoints=' + TestUtil.protocol + ' -p 12347 -t 5000"' + \
            r' -e "add \"test' + secure + ':' + TestUtil.protocol + r' -p 12345 -t 5000\"' + \
            ' ' + server + updatedServerOptions + '"'
    print "registering server with icepack for automatic activation...",
    icePackAdminPipe = os.popen(command)
    icePackAdminPipe.close()
    print "ok"

    print "starting client...",
    exit
    clientPipe = os.popen(client + updatedClientOptions)
    output = clientPipe.readline()
    if not output:
        print "failed!"
        TestUtil.killServers()
        sys.exit(1)
    print "ok"
    print output,
    while 1:
        output = clientPipe.readline()
        if not output:
            break;
        print output,

command = icePackAdmin + updatedClientOptions + \
          r' "--Ice.Adapter.Admin.Endpoints=' + TestUtil.protocol + r' -p 12347 -t 5000"' + r' -e "shutdown"'
print "shutting down icepack...",
icePackAdminPipe = os.popen(command)
icePackAdminPipe.close()
print "ok"

sys.exit(0)
