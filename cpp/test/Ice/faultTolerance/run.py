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

testdir = os.path.normpath(toplevel + "/test/Ice/faultTolerance")
server = os.path.normpath(testdir + "/server")
client = os.path.normpath(testdir + "/client")

num = 8
base = 12340

serverPipes = { }
updatedServerOptions = TestUtil.serverOptions
updatedServerOptions = updatedServerOptions.replace("TOPLEVELDIR", toplevel)
for i in range(0, num):
    print "starting server #%d..." % (i + 1),
    serverPipes[i] = os.popen(server + updatedServerOptions + " %d" % (base + i))
    TestUtil.getServerPid(serverPipes[i])
    TestUtil.getAdapterReady(serverPipes[i])
    print "ok"

updatedClientOptions = TestUtil.clientOptions
updatedClientOptions = updatedClientOptions.replace("TOPLEVELDIR", toplevel)

ports = ""
for i in range(0, num):
    ports = "%s %d" % (ports, base + i)
print "starting client...",
clientPipe = os.popen(client + " " + updatedClientOptions + " " + ports)
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

sys.exit(0)
