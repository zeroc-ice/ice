#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

name = os.path.join("Ice", "exceptions")
testdir = os.path.join(toplevel, "test", name)
nameAMD = os.path.join("Ice", "exceptionsAMD")
testdirAMD = os.path.join(toplevel, "test", nameAMD)

print "tests with regular server."
classpath = os.getenv("CLASSPATH", "")
os.environ["CLASSPATH"] = os.path.join(testdir, "classes") + TestUtil.sep + classpath
TestUtil.clientServerTestWithOptions("", " --Ice.Warn.Connections=0")

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
(clientPipeIn, clientPipe) = os.popen4(client + TestUtil.clientOptions + " --Ice.Warn.Connections=0")
print "ok"
TestUtil.printOutputFromPipe(clientPipe)
clientStatus = clientPipe.close()
serverStatus = serverPipe.close()
if clientStatus or serverStatus:
    TestUtil.killServers()
    sys.exit(1)

print "tests with collocated server."
TestUtil.collocatedTest()
sys.exit(0)
