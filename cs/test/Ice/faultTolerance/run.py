#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, getopt

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

server = os.path.join(testdir, "server")
client = os.path.join(testdir, "client")

if TestUtil.isCygwin():
    print "\nYou may get spurious \"Signal 127\" messages during this test run."
    print "These are expected and can be safely ignored.\n"

num = 12
base = 12340

for i in range(0, num):
    msg = "starting "
    msg = "server"
    msg += " #%d..." % (i + 1)
    print msg,
    serverPipe = os.popen(TestUtil.createCmd(server) + TestUtil.serverOptions + " %d" % (base + i))
    TestUtil.getServerPid(serverPipe)
    TestUtil.getAdapterReady(serverPipe)
    print "ok"

ports = ""
for i in range(0, num):
    ports = "%s %d" % (ports, base + i)

print TestUtil.createMsg("client"),
clientPipe = os.popen(TestUtil.createCmd(client) + TestUtil.clientOptions + " " + ports)
print "ok"

TestUtil.printOutputFromPipe(clientPipe)

clientStatus = TestUtil.closePipe(clientPipe)

if clientStatus:
    sys.exit(1)

#
# We simuluate the abort of the server by calling Process.Kill(). However, this
# results in a non-zero exit status. Therefore we ignore the status.
#
#if TestUtil.serverStatus():
    #sys.exit(1)
TestUtil.joinServers()

sys.exit(0)
