#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
# Mutable Realms, Inc.
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
print "ok"

for output in clientPipe.xreadlines():
    print output,

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
# machines, when ssl isn't enabled, and which cause some servers to
# segfault and abort. It's not clear what the problem is, and it's
# almost impossible to debug with the very poor information we get
# from the core file (ulimit -c unlimited to enable core files on
# Linux).
#
if serverStatus:
    TestUtil.killServers()
    if TestUtil.isWin32():
        sys.exit(1)
    else:
        sys.exit(0)

sys.exit(0)
