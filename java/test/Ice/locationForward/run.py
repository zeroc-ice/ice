#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
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

testdir = os.path.join(toplevel, "test", "Ice", "locationForward")
classpath = os.path.join(toplevel, "lib") + TestUtil.sep + os.path.join(testdir, "classes") + \
    TestUtil.sep + os.getenv("CLASSPATH", "")
server = "java -classpath \"" + classpath + "\" Server"
client = "java -classpath \"" + classpath + "\" Client"

num = 5
base = 12340

updatedServerOptions = TestUtil.serverOptions.replace("TOPLEVELDIR", toplevel)
updatedClientOptions = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel)

serverPipes = { }
for i in range(0, num):
    if i + 1 < num:
        s = " --fwd \"test:default -t 2000 -p %d\" %d" % ((base + i + 1), (base + i))
    else:
        s = " %d" % (base + i)
    print "starting server #%d..." % (i + 1),
    command = server + updatedServerOptions + s;
    serverPipes[i] = os.popen(command)
    TestUtil.getAdapterReady(serverPipes[i])
    print "ok"

print "starting client...",
s = " %d %d" % (base, (base + num - 1))
command = client + updatedClientOptions + s
clientPipe = os.popen(command)
print "ok"

for output in clientPipe.xreadlines():
    print output,

clientStatus = clientPipe.close()
serverStatus = None
for i in range(0, num):
    serverStatus = serverStatus or serverPipes[i].close()

if clientStatus or serverStatus:
    sys.exit(1)

sys.exit(0)
