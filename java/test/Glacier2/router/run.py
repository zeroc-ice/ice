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

if not os.environ.has_key('ICE_HOME'):
    print "ICE_HOME is not defined."
    sys.exit(0)

ice_home = os.environ['ICE_HOME']

router = os.path.join(ice_home, "bin", "glacier2router")

command = router + TestUtil.cppClientServerOptions + \
          r' --Ice.Override.Compress=0' \
          r' --Ice.PrintProcessId' \
          r' --Glacier2.AllowCategories="c1 c2"' + \
          r' --Glacier2.AddUserToAllowCategories="2"' + \
          r' --Glacier2.RouterIdentity="abc/def"' + \
          r' --Glacier2.AdminIdentity="ABC/DEF"' + \
          r' --Glacier2.Client.Endpoints="default -z -p 12347 -t 10000"' + \
          r' --Glacier2.Server.Endpoints="tcp -z -h 127.0.0.1 -t 10000"' \
          r' --Glacier2.Admin.Endpoints="tcp -z -h 127.0.0.1 -p 12348 -t 10000"' + \
          r' --Glacier2.CryptPasswords="' + toplevel + r'/test/Glacier2/router/passwords"'

print "starting router...",
starterPipe = os.popen(command)
TestUtil.getServerPid(starterPipe)
TestUtil.getAdapterReady(starterPipe)
print "ok"

name = os.path.join("Glacier2", "router")
testdir = os.path.join(toplevel, "test", name)
os.environ["CLASSPATH"] = os.path.join(testdir, "classes") + TestUtil.sep + os.getenv("CLASSPATH", "")

TestUtil.mixedClientServerTest()

#
# We run the test again, to check whether the glacier router can
# handle multiple clients. Also, when we run for the second time, we
# want the client to shutdown the router after running the tests.
#
TestUtil.mixedClientServerTestWithOptions("", " --shutdown")

starterStatus = starterPipe.close()

if starterStatus:
    TestUtil.killServers()
    sys.exit(1)

sys.exit(0)
