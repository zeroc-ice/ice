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

name = os.path.join("Freeze", "cursor")
testdir = os.path.join(toplevel, "test", name)

dbdir = os.path.join(testdir, "db")
TestUtil.cleanDbDir(dbdir)

client = os.path.join(testdir, "client")
clientOptions = ' ' + testdir;

print "starting client...",
clientPipe = os.popen(client + clientOptions)
print "ok"

for output in clientPipe.xreadlines():
    print output,

clientStatus = clientPipe.close()

if clientStatus:
    sys.exit(1)

sys.exit(0)
