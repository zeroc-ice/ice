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

testdir = os.path.normpath(toplevel + "/test/Ice/locationForward")
server = os.path.normpath(testdir + "/server")
client = os.path.normpath(testdir + "/client")

num = 5
base = 12340

serverPipes = { }
for i in range(0, num):
    print "starting server #%d..." % (i + 1),
    if i + 1 < num:
        s = TestUtil.serverOptions + " --fwd \"test:tcp -t 2000 -p %d\" %d" \
            % ((base + i + 1), (base + i))
    else:
        s = TestUtil.serverOptions + " %d" % (base + i)
    serverPipes[i] = os.popen(server + " " + s)
    TestUtil.getServerPid(serverPipes[i])
    TestUtil.getAdapterReady(serverPipes[i])
    print "ok"

print "starting client...",
s = "%d %d" % (base, (base + num - 1))
clientPipe = os.popen(client + " " + s)
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
