#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
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

testOptions = " --IceSSL.Client.CertPath=" + os.path.join(toplevel, "test", "IceSSL", "certs") + \
              " --IceSSL.Client.Config= " + \
              " --IceSSL.Server.CertPath=" + os.path.join(toplevel, "test", "IceSSL", "certs") + \
              " --IceSSL.Server.Config= "

name = os.path.join("IceSSL", "loadPEM")
testdir = os.path.join(toplevel, "test", name)

client = os.path.join(testdir, "loadPEM")

localClientOptions = TestUtil.clientServerProtocol + TestUtil.defaultHost
print "starting loadPEM...",
clientPipe = os.popen(client + localClientOptions + testOptions + " 2>&1")
print "ok"

TestUtil.printOutputFromPipe(clientPipe)
    
clientStatus = clientPipe.close()

if clientStatus:
    sys.exit(1)

sys.exit(0)
