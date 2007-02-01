#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
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

name = os.path.join("Ice", "faultTolerance")
testdir = os.path.join(toplevel, "test", name)

cwd = os.getcwd()
os.chdir(testdir)

server = "Server.py"
client = "Client.py"

num = 12
base = 12340

for i in range(0, num):
    print "starting server #%d..." % (i + 1),
    sys.stdout.flush()
    command = "python " + server + TestUtil.serverOptions + " %d" % (base + i)
    if TestUtil.debug:
        print "(" + command + ")",
    serverPipe = os.popen(command + " 2>&1")
    TestUtil.getServerPid(serverPipe)
    TestUtil.getAdapterReady(serverPipe)
    print "ok"

ports = ""
for i in range(0, num):
    ports = "%s %d" % (ports, base + i)
print "starting client...",
command = "python " + client + TestUtil.clientOptions + " " + ports
if TestUtil.debug:
    print "(" + command + ")",
clientPipe = os.popen(command + " 2>&1")
print "ok"

TestUtil.printOutputFromPipe(clientPipe)
clientStatus = TestUtil.closePipe(clientPipe)
if clientStatus:
    TestUtil.killServers()

if clientStatus or TestUtil.serverStatus():
    sys.exit(1)

sys.exit(0)
