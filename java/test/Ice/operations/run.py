#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2002
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

name = os.path.join("Ice", "operations")
testdir = os.path.join(toplevel, "test", name)
nameAMD = os.path.join("Ice", "operationsAMD")
testdirAMD = os.path.join(toplevel, "test", nameAMD)

print "tests with regular server."
classpath = os.getenv("CLASSPATH", "")
os.environ["CLASSPATH"] = os.path.join(testdir, "classes") + TestUtil.sep + classpath
TestUtil.clientServerTest()

print "tests with AMD server."
server = "java -ea Server --Ice.ProgramName=Server "
client = "java -ea Client --Ice.ProgramName=Client "
print "starting server...",
classpath = os.getenv("CLASSPATH", "")
os.environ["CLASSPATH"] = os.path.join(testdirAMD, "classes") + TestUtil.sep + classpath
serverPipe = os.popen(server + TestUtil.serverOptions)
TestUtil.getAdapterReady(serverPipe)
print "ok"
print "starting client...",
classpath = os.getenv("CLASSPATH", "")
os.environ["CLASSPATH"] = os.path.join(testdir, "classes") + TestUtil.sep + classpath
clientPipe = os.popen(client + TestUtil.clientOptions)
print "ok"
for output in clientPipe.xreadlines():
    print output,
clientStatus = clientPipe.close()
serverStatus = serverPipe.close()
if clientStatus or serverStatus:
    TestUtil.killServers()
    sys.exit(1)

print "tests with collocated server."
TestUtil.collocatedTest()
sys.exit(0)
