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
    if os.path.exists(os.path.normpath(toplevel + "/config/TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.normpath(toplevel + "/config"))
import TestUtil

icePack = os.path.normpath(toplevel + "/bin/icepack")
icePackAdmin = os.path.normpath(toplevel + "/bin/icepackadmin")

updatedServerOptions = TestUtil.serverOptions
updatedServerOptions = updatedServerOptions.replace("TOPLEVELDIR", toplevel)
updatedClientOptions = TestUtil.clientOptions
updatedClientOptions = updatedClientOptions.replace("TOPLEVELDIR", toplevel)

command = icePack + ' ' + updatedServerOptions + ' --nowarn' + \
       r' "--Ice.Adapter.Forward.Endpoints=' + TestUtil.protocol + ' -p 12346 -t 5000"' + \
       r' "--Ice.Adapter.Admin.Endpoints=' + TestUtil.protocol + ' -p 12347 -t 5000"'
print "starting icepack...",
icePackPipe = os.popen(command)
TestUtil.getServerPid(icePackPipe)
TestUtil.getAdapterReady(icePackPipe)
TestUtil.getAdapterReady(icePackPipe)
print "ok"

secure = " -s "
if TestUtil.protocol == "ssl":
    secure = " -s "

command = icePackAdmin + ' ' + updatedClientOptions + \
        r' "--Ice.Adapter.Admin.Endpoints=' + TestUtil.protocol + ' -p 12347 -t 5000"' + \
        r' -e "add \"test' + secure + ':' + TestUtil.protocol + r' -p 12345 -t 5000\" " '
print "registering server with icepack...",
icePackAdminPipe = os.popen(command)
icePackAdminPipe.close()
print "ok"

name = "IcePack/simple"
TestUtil.clientServerTest(toplevel, name)
TestUtil.collocatedTest(toplevel, name)

if os.name != "nt":
    testdir = os.path.normpath(toplevel + "/test/IcePack/simple")

    server = os.path.normpath(testdir + "/server")
    server = server + " " + updatedServerOptions

    client = os.path.normpath(testdir + "/client")
    client = client + " " + updatedClientOptions

    command = icePackAdmin + ' ' + updatedClientOptions + \
            r' "--Ice.Adapter.Admin.Endpoints=' + TestUtil.protocol + ' -p 12347 -t 5000"' + \
            r' -e "add \"test' + secure + ':' + TestUtil.protocol + r' -p 12345 -t 5000\" \"' + server + \
            r'"\"'
    print "registering server with icepack for automatic activation...",
    icePackAdminPipe = os.popen(command)
    icePackAdminPipe.close()
    print "ok"

    print "starting client...",
    clientPipe = os.popen(client)
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

command = icePackAdmin + ' ' + updatedClientOptions + \
        r' "--Ice.Adapter.Admin.Endpoints=' + TestUtil.protocol + r' -p 12347 -t 5000"' + \
        r' -e "shutdown"'
print "shutting down icepack...",
icePackAdminPipe = os.popen(command)
icePackAdminPipe.close()
print "ok"

sys.exit(0)
