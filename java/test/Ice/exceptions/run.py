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

name = os.path.join("Ice", "exceptions")
testdir = os.path.join(toplevel, "test", name)
nameAMD = os.path.join("Ice", "exceptionsAMD")
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
(serverPipeIn, serverPipe) = os.popen4(server + TestUtil.serverOptions)
TestUtil.getAdapterReady(serverPipe)
print "ok"
print "starting client...",
classpath = os.getenv("CLASSPATH", "")
os.environ["CLASSPATH"] = os.path.join(testdir, "classes") + TestUtil.sep + classpath
(clientPipeIn, clientPipe) = os.popen4(client + TestUtil.clientOptions)
print "ok"
TestUtil.printOutputFromPipe(clientPipe)
clientInStatus = clientPipeIn.close()
clientStatus = clientPipe.close()
serverInStatus = serverPipeIn.close()
serverStatus = serverPipe.close()
if clientInStatus or serverInStatus:
    TestUtil.killServers()
    sys.exit(1)

print "tests with collocated server."
TestUtil.collocatedTest()
sys.exit(0)
