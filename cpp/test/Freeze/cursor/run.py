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
    if os.path.exists(os.path.normpath(toplevel + "/config/TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.normpath(toplevel + "/config"))
import TestUtil

name = "Freeze/cursor"

testdir = os.path.normpath(toplevel + "/test/" + name)
client = os.path.normpath(testdir + "/client")

print "starting client...",
clientPipe = os.popen(client)
output = clientPipe.read().strip()
if not output:
    print "failed!"
    sys.exit(1)
print "ok"
print output

sys.exit(0)
