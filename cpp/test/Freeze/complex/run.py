#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
# Mutable Realms, Inc.
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

name = os.path.join("Freeze", "complex")

testdir = os.path.join(toplevel, "test", name)

#
# Clean the contents of the database directory.
#
dbdir = os.path.join(testdir, "db")
TestUtil.cleanDbDir(dbdir)

client = os.path.join(testdir, "client")
clientOptions = " --dbdir " + testdir;

print "starting populate...",
populatePipe = os.popen(client + clientOptions + " populate")
print "ok"

for output in populatePipe.xreadlines():
    print output,

populateStatus = populatePipe.close()

if populateStatus:
    sys.exit(1)

print "starting verification client...",
clientPipe = os.popen(client + clientOptions + " validate")
print "ok"

for output in clientPipe.xreadlines():
    print output,

clientStatus = clientPipe.close()

if clientStatus:
    sys.exit(1)

sys.exit(0)
