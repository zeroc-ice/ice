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

if TestUtil.protocol != "ssl":
    print "This test may only be run with SSL enabled."
    sys.exit(0)

testdir = os.path.join(toplevel,"test", "IceSSL", "configuration")
client = os.path.join(testdir, "configuration")

localClientOptions = TestUtil.clientServerProtocol + TestUtil.defaultHost
updatedOptions = localClientOptions.replace("TOPLEVELDIR", toplevel)
print "starting configuration...",
clientPipe = os.popen(client + updatedOptions)
print "ok"

for output in clientPipe.xreadlines():
    print output,
    
clientStatus = clientPipe.close()

if clientStatus:
    sys.exit(1)

sys.exit(0)
