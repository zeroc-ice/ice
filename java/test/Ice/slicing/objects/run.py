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

name = os.path.join("Ice", "slicing", "objects")
testdir = os.path.join(toplevel, "test", name)
nameAMD = os.path.join("Ice", "slicing", "objectsAMD")
testdirAMD = os.path.join(toplevel, "test", nameAMD)

print "tests with regular server."
TestUtil.clientServerTestWithClasspath(os.path.join(testdir, "sclasses"), os.path.join(testdir, "cclasses"))

print "tests with AMD server."
server = "java -ea Server --Ice.ProgramName=Server "
client = "java -ea Client --Ice.ProgramName=Client "
print "starting server...",
cp = os.getenv("CLASSPATH", "")
os.environ["CLASSPATH"] = os.path.join(testdirAMD, "classes") + TestUtil.sep + cp
serverPipe = os.popen(server + TestUtil.serverOptions)
TestUtil.getAdapterReady(serverPipe)
print "ok"
print "starting client...",
os.environ["CLASSPATH"] = cp
os.environ["CLASSPATH"] = os.path.join(testdir, "cclasses") + TestUtil.sep + cp
clientPipe = os.popen(client + TestUtil.clientOptions)
print "ok"
TestUtil.printOutputFromPipe(clientPipe)
clientStatus = clientPipe.close()
serverStatus = serverPipe.close()
if clientStatus or serverStatus:
    TestUtil.killServers()
    sys.exit(1)

sys.exit(0)
