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

name = os.path.join("IceXML", "encoding")

testdir = os.path.join(toplevel, "test", name)

client = os.path.join(testdir, "client" + ' ' + testdir)

print "starting client...",
clientPipe = os.popen(client)
output = clientPipe.read().strip()
if not output:
    print "failed!"
    sys.exit(1)
print "ok"
print output

sys.exit(0)
