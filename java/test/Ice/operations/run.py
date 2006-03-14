#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
serverPipe = os.popen(server + TestUtil.serverOptions + " 2>&1")
TestUtil.getAdapterReady(serverPipe)
print "ok"
print "starting client...",
classpath = os.getenv("CLASSPATH", "")
os.environ["CLASSPATH"] = os.path.join(testdir, "classes") + TestUtil.sep + classpath
clientPipe = os.popen(client + TestUtil.clientOptions + " 2>&1")
print "ok"
TestUtil.printOutputFromPipe(clientPipe)
clientStatus = TestUtil.closePipe(clientPipe)
serverStatus = TestUtil.closePipe(serverPipe)
if clientStatus or serverStatus:
    TestUtil.killServers()
    sys.exit(1)

print "tests with collocated server."
TestUtil.collocatedTest()
sys.exit(0)
