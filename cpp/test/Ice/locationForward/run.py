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

testdir = os.path.join(toplevel, "test", "Ice", "locationForward")
server = os.path.join(testdir, "server")
client = os.path.join(testdir, "client")

num = 5
base = 12340

updatedServerOptions = TestUtil.serverOptions.replace("TOPLEVELDIR", toplevel)
updatedClientOptions = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel)

if TestUtil.protocol == "ssl":
    secure = " -s "
else:
    secure = ""

serverPipes = { }
for i in range(0, num):
    if i + 1 < num:
        s = " --fwd \"test" + secure + ":" + TestUtil.protocol + " -t 2000 -p %d\" %d" \
            % ((base + i + 1), (base + i))
    else:
        s = " %d" % (base + i)
    print "starting server #%d..." % (i + 1),
    serverPipes[i] = os.popen(server + updatedServerOptions + s)
    TestUtil.getServerPid(serverPipes[i])
    TestUtil.getAdapterReady(serverPipes[i])
    print "ok"

print "starting client...",
s = " %d %d" % (base, (base + num - 1))
clientPipe = os.popen(client + updatedClientOptions + s)
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
