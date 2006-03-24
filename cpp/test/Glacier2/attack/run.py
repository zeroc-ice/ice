#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

sys.stdout.write("Testing add proxy rules...")
testcase = file("./test.dat")
lines = testcase.readlines()
input = os.popen(r"ruletester 1>test.out 2>test.log", "w")
for l in lines:
    input.write(l)

if not input.close() == None: 
    print "There were errors, check test.log"
else:
    print "ok"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

router = os.path.join(toplevel, "bin", "glacier2router")

command = router + TestUtil.clientServerOptions + \
          r' --Glacier2.RoutingTable.MaxSize=10' + \
          r' --Glacier2.Client.Endpoints="default -p 12347 -t 10000"' + \
          r' --Glacier2.Admin.Endpoints="tcp -h 127.0.0.1 -p 12348 -t 10000"' + \
          r' --Glacier2.CryptPasswords="' + toplevel + r'/test/Glacier2/attack/passwords"'

print "starting router...",
starterPipe = os.popen(command)
TestUtil.getServerPid(starterPipe)
TestUtil.getAdapterReady(starterPipe)
print "ok"

name = os.path.join("Glacier2", "attack")

TestUtil.mixedClientServerTest(name)

starterStatus = TestUtil.closePipe(starterPipe)

if starterStatus:
    TestUtil.killServers()
    sys.exit(1)

sys.exit(0)
