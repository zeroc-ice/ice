#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

def usage():
    print "usage: " + sys.argv[0] + " [-m]"
    sys.exit(2)

try:
    opts, args = getopt.getopt(sys.argv[1:], "m")
except getopt.GetoptError:
    usage()

mono = 0
for o, a in opts:
    if o == "-m":
        mono = 1

if not TestUtil.isWin32():
    mono = 1
    
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
    if mono:
        msg += "mono "
    msg += "server"
    if mono:
        msg += ".exe"
    msg += " #%d..." % (i + 1)
    print msg,
    serverPipe = os.popen(TestUtil.createCmd(mono, server) + TestUtil.serverOptions + " %d" % (base + i))
    TestUtil.getServerPid(serverPipe)
    TestUtil.getAdapterReady(serverPipe)
    print "ok"

ports = ""
for i in range(0, num):
    ports = "%s %d" % (ports, base + i)

print TestUtil.createMsg(mono, "client"),
clientPipe = os.popen(TestUtil.createCmd(mono, client) + TestUtil.clientOptions + " " + ports)
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
