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

name = os.path.join("Freeze", "dbmap")
testdir = os.path.join(toplevel, "test", name)
os.environ["CLASSPATH"] = os.path.join(testdir, "classes") + TestUtil.sep + os.getenv("CLASSPATH", "")

dbdir = os.path.join(testdir, "db")
TestUtil.cleanDbDir(dbdir)

client = "java -ea Client"

print "starting client...",
(clientPipeIn, clientPipe) = os.popen4(client + TestUtil.clientOptions + " " + testdir)
print "ok"

TestUtil.printOutputFromPipe(clientPipe)

clientInStatus = clientPipeIn.close()
clientStatus = clientPipe.close()

if clientInStatus or clientStatus:
    sys.exit(1)

sys.exit(0)
