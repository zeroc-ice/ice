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

print "starting icepack...",
icePackPipe = os.popen(icePack + ' --Ice.PrintProcessId --nowarn' + \
                       r' "--Ice.Adapter.Forward.Endpoints=tcp -p 12346 -t 5000"' + \
                       r' "--Ice.Adapter.Admin.Endpoints=tcp -p 12347 -t 5000"')
output = icePackPipe.readline().strip()
if not output:
    print "failed!"
    sys.exit(0)
TestUtil.serverPids.append(int(output))
print "ok"

print "registering server with icepack...",
icePackAdminPipe = os.popen(icePackAdmin + \
                            r' "--Ice.Adapter.Admin.Endpoints=tcp -p 12347 -t 5000"' + \
                            r' -e "add \"test:tcp -p 12345 -t 5000\""')
icePackAdminPipe.close()
print "ok"

name = "IcePack/simple"
TestUtil.clientServerTest(toplevel, name)
TestUtil.collocatedTest(toplevel, name)

if os.name != "nt":
    testdir = os.path.normpath(toplevel + "/test/IcePack/simple")
    server = os.path.normpath(testdir + "/server")
    client = os.path.normpath(testdir + "/client")

    print "registering server with icepack for automatic activation...",
    icePackAdminPipe = os.popen(icePackAdmin + \
                                r' "--Ice.Adapter.Admin.Endpoints=tcp -p 12347 -t 5000"' + \
                                r' -e "add \"test:tcp -p 12345 -t 5000\" \"' + server + r'\""')
    icePackAdminPipe.close()
    print "ok"

    print "starting client...",
    clientPipe = os.popen(client)
    output = clientPipe.readline()
    if not output:
        print "failed!"
        TestUtil.killServers()
        sys.exit(0)
    print "ok"
    print output,
    while 1:
        output = clientPipe.readline()
        if not output:
            break;
        print output,
    
print "shutting down icepack...",
icePackAdminPipe = os.popen(icePackAdmin + \
                            r' "--Ice.Adapter.Admin.Endpoints=tcp -p 12347 -t 5000"' + \
                            r' -e "shutdown"')
icePackAdminPipe.close()
print "ok"

sys.exit(1)
