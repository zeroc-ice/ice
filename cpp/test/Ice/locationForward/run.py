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

for toplevel in ["", "..", os.path.join("..", ".."), os.path.join("..", "..", "..")]:
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

name = os.path.join("Ice", "locationForward")

testdir = os.path.join(toplevel, "test", name)
server = os.path.join(testdir, "server")
client = os.path.join(testdir, "client")

num = 5
base = 12340

serverPipes = { }
for i in range(0, num):
    print "starting server #%d..." % (i + 1),
    if i + 1 < num:
        s = "server --pid --fwd \"test:tcp -t 2000 -p %d\" %d" % ((base + i + 1), (base + i))
    else:
        s = "server --pid %d" % (base + i)
    serverPipes[i] = os.popen(os.path.join(testdir, s))
    output = serverPipes[i].readline().strip()
    if not output:
        print "failed!"
        sys.exit(0)
    TestUtil.serverPids.append(int(output))
    print "ok"

print "starting client...",
s = "client %d %d" % (base, (base + num - 1))
clientPipe = os.popen(os.path.join(testdir, s))
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
