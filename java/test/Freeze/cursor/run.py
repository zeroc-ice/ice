#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
# ZeroC, Inc.
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

testdir = os.path.join(toplevel,"test", "Freeze", "cursor")

dbdir = os.path.join(testdir, "db")
TestUtil.cleanDbDir(dbdir)

classpath = os.path.join(toplevel, "lib") + TestUtil.sep + os.path.join(testdir, "classes") + \
    TestUtil.sep + os.getenv("CLASSPATH", "")
client = "java -classpath \"" + classpath + "\" Client " + testdir

print "starting client...",
clientPipe = os.popen(client)
print "ok"

for output in clientPipe.xreadlines():
    print output,

clientStatus = clientPipe.close()

if clientStatus:
    sys.exit(1)

sys.exit(0)
