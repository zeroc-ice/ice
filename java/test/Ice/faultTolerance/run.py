#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
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
os.environ["CLASSPATH"] = os.path.join(testdir, "classes") + TestUtil.sep + os.getenv("CLASSPATH", "")

server = "java -ea Server"
client = "java -ea Client"

num = 12
base = 12340

serverPipes = { }
serverPipesIn = { }
for i in range(0, num):
    print "starting server #%d..." % (i + 1),
    (serverPipesIn[i], serverPipes[i]) = os.popen4(server + TestUtil.serverOptions + " %d" % (base + i))
    TestUtil.getAdapterReady(serverPipes[i])
    print "ok"

ports = ""
for i in range(0, num):
    ports = "%s %d" % (ports, base + i)
print "starting client...",
(clientPipeIn, clientPipe) = os.popen4(client + TestUtil.clientOptions + " " + ports)
print "ok"

TestUtil.printOutputFromPipe(clientPipe)

clientInStatus = clientPipeIn.close()
clientStatus = clientPipe.close()
serverStatus = None
for i in range(0, num):
    serverStatus = serverStatus or serverPipes[i].close() or serverPipesIn[i].close()

if clientInStatus or clientStatus or serverStatus:
    sys.exit(1)

sys.exit(0)
