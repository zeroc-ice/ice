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

if TestUtil.protocol != "ssl":
    print "This test may only be run with SSL enabled."
    sys.exit(0)

testdir = os.path.join(toplevel,"test", "IceSSL", "certificateVerifier")
client = os.path.join(testdir, "certificateVerifier")

updatedOptions = TestUtil.clientServerOptions.replace("TOPLEVELDIR", toplevel)
print "starting certificateVerifier...",
clientPipe = os.popen(client + updatedOptions)
output = clientPipe.read().strip()
if not output:
    print "failed!"
    sys.exit(1)
print "ok"
print output
sys.exit(0)
