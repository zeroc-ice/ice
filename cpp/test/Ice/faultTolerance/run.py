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

testdir = os.path.join(toplevel,"test", "Ice", "faultTolerance")
server = os.path.join(testdir, "server")
client = os.path.join(testdir, "client")

num = 8
base = 12340

updatedServerOptions = TestUtil.serverOptions.replace("TOPLEVELDIR", toplevel)
updatedClientOptions = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel)

serverPipes = { }
for i in range(0, num):
    print "starting server #%d..." % (i + 1),
    serverPipes[i] = os.popen(server + updatedServerOptions + " %d" % (base + i))
    TestUtil.getServerPid(serverPipes[i])
    TestUtil.getAdapterReady(serverPipes[i])
    print "ok"

ports = ""
for i in range(0, num):
    ports = "%s %d" % (ports, base + i)
print "starting client...",
clientPipe = os.popen(client + updatedClientOptions + " " + ports)
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
