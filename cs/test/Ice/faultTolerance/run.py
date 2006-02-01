#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

serverPipes = { }
for i in range(0, num):
    msg = "starting "
    if mono:
        msg += "mono "
    msg += "server"
    if mono:
        msg += ".exe"
    msg += " #%d..." % (i + 1)
    print msg,
    serverPipes[i] = os.popen(TestUtil.createCmd(mono, server) + TestUtil.serverOptions + " %d" % (base + i))
    TestUtil.getServerPid(serverPipes[i])
    TestUtil.getAdapterReady(serverPipes[i])
    print "ok"

ports = ""
for i in range(0, num):
    ports = "%s %d" % (ports, base + i)

print TestUtil.createMsg(mono, "client"),
clientPipe = os.popen(TestUtil.createCmd(mono, client) + TestUtil.clientOptions + " " + ports)
print "ok"

TestUtil.printOutputFromPipe(clientPipe)

clientStatus = TestUtil.closePipe(clientPipe)
serverStatus = None

#
# With ActiveState Python, the close() call raises an IOError if
# the server has died (whereas with Cygwin Python, no exception
# is raised. We swallow IOError here to avoid having the test
# claim that it failed when in fact it succeeded.
#
#for i in range(0, num):
#    try:
#	serverStatus = serverStatus or serverPipes[i].close()
#    except IOError, error:
#	pass

for i in range(0, num):
    serverStatus = serverStatus or TestUtil.closePipe(serverPipes[i])

if clientStatus:
    TestUtil.killServers()
    sys.exit(1)

#
# Exit with status 0 even though some servers failed to shutdown
# properly. There's a problem which is occuring on Linux dual-processor
# machines, when ssl isn't enabled, and which cause some servers to
# segfault and abort. It's not clear what the problem is, and it's
# almost impossible to debug with the very poor information we get
# from the core file (ulimit -c unlimited to enable core files on
# Linux).
#
# For C#, we also have the problem that terminating a server with
# Process.Kill() is the only way to simulate an abort, but the
# server then returns non-zero exit status.
#
if serverStatus:
    TestUtil.killServers()
#    sys.exit(1)
#    if TestUtil.isWin32():
#        sys.exit(1)
#    else:
#        sys.exit(0)

sys.exit(0)
