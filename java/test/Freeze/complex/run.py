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

name = os.path.join("Freeze", "complex")
testdir = os.path.join(toplevel, "test", name)
os.environ["CLASSPATH"] = os.path.join(testdir, "classes") + TestUtil.sep + os.environ["CLASSPATH"]

#
# Clean the contents of the database directory.
#
dbdir = os.path.join(testdir, "db")
TestUtil.cleanDbDir(dbdir)

client = "java -ea Client"

print "starting populate...",
populatePipe = os.popen(client + TestUtil.clientOptions + " --dbdir " + testdir + " populate")
print "ok"

for output in populatePipe.xreadlines():
    print output,

populateStatus = populatePipe.close()

if populateStatus:
    sys.exit(1)

print "starting verification client...",
clientPipe = os.popen(client + TestUtil.clientOptions + " --dbdir " + testdir + " validate")
print "ok"

for output in clientPipe.xreadlines():
    print output,

clientStatus = clientPipe.close()

if clientStatus:
    sys.exit(1)

sys.exit(0)
