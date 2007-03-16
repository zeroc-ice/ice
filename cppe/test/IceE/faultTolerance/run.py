#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
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

name = os.path.join("IceE", "faultTolerance")
testdir = os.path.join(toplevel, "test", name)

server = os.path.join(testdir, "server")
client = os.path.join(testdir, "client")

num = 12
base = 12340

serverPipes = { }
for i in range(0, num):
    print "starting server #%d..." % (i + 1),
    serverPipes[i] = os.popen(server + TestUtil.serverOptions + " %d" % (base + i) + " 2>&1")
    TestUtil.getServerPid(serverPipes[i])
    TestUtil.getAdapterReady(serverPipes[i])
    print "ok"

ports = ""
for i in range(0, num):
    ports = "%s %d" % (ports, base + i)
print "starting client...",
clientPipe = os.popen(client + TestUtil.clientOptions + " " + ports + " 2>&1")
print "ok"

TestUtil.printOutputFromPipe(clientPipe)

clientStatus = clientPipe.close()
serverStatus = None
for i in range(0, num):
    serverStatus = serverStatus or serverPipes[i].close()

if clientStatus:
    TestUtil.killServers()
    sys.exit(1)

#
# Exit with status 0 even though some servers failed to shutdown
# properly. There's a problem which is occuring on Linux dual-processor
# machines, and which cause some servers to
# segfault and abort. It's not clear what the problem is, and it's
# almost impossible to debug with the very poor information we get
# from the core file (ulimit -c unlimited to enable core files on
# Linux).
#
if serverStatus:
    TestUtil.killServers()
    sys.exit(1)
#    if TestUtil.isWin32():
#        sys.exit(1)
#    else:
#        sys.exit(0)

sys.exit(0)
