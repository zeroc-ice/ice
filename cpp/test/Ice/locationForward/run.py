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
        s = "--Ice.PrintProcessId --Ice.PrintAdapterReady --fwd \"test:tcp -t 2000 -p %d\" %d" \
            % ((base + i + 1), (base + i))
    else:
        s = "--Ice.PrintProcessId --Ice.PrintAdapterReady %d" % (base + i)
    serverPipes[i] = os.popen(server + " " + s)
    output = serverPipes[i].readline().strip()
    if not output:
        print "failed!"
        sys.exit(0)
    TestUtil.serverPids.append(int(output))
    output = serverPipes[i].readline().strip()
    if not output:
        print "failed!"
        sys.exit(0)
    print "ok"

print "starting client...",
s = "%d %d" % (base, (base + num - 1))
clientPipe = os.popen(client + " " + s)
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

sys.exit(1)
