#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
# MutableRealms, Inc.
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
    TestUtil.sep + os.environ['CLASSPATH']
client = "java -classpath \"" + classpath + "\" Client " + testdir

print "starting client...",
clientPipe = os.popen(client)
output = clientPipe.read().strip()
if not output:
    print "failed!"
    clientPipe.close()
    sys.exit(1)
print "ok"
print output

clientStatus = clientPipe.close()

if clientStatus:
    sys.exit(1)

sys.exit(0)
