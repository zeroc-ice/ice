#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, os.path

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil
TestUtil.processCmdLine() 

testdir = os.path.dirname(os.path.abspath(__file__))

#
# Clean the contents of the database directory.
#
dbdir = os.path.join(testdir, "db")
TestUtil.cleanDbDir(dbdir)

client = os.path.join(testdir, "client")

print "starting populate...",
populatePipe = TestUtil.startClient(client, " --dbdir " + testdir + " populate" + " 2>&1")
print "ok"

TestUtil.printOutputFromPipe(populatePipe)

populateStatus = TestUtil.closePipe(populatePipe)

if populateStatus:
    sys.exit(1)

print "starting verification client...",
clientPipe = TestUtil.startClient(client, " --dbdir " + testdir + " validate" + " 2>&1")
print "ok"

TestUtil.printOutputFromPipe(clientPipe)

clientStatus = TestUtil.closePipe(clientPipe)

if clientStatus:
    sys.exit(1)

sys.exit(0)
