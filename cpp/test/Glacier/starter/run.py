#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
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

starter = os.path.join(toplevel, "bin", "glacierstarter")
router = os.path.join(toplevel, "bin", "glacierrouter")

command = starter + TestUtil.clientServerOptions + \
          r' --Glacier.Starter.RouterPath=' + router + \
          r' --Glacier.Starter.PropertiesOverride=Ice.ServerIdleTime=10' \
          r' --Glacier.Starter.CryptPasswords="' + toplevel + r'/test/Glacier/starter/passwords"' + \
          r' --Glacier.Starter.Endpoints="default -p 12346 -t 30000"' + \
          r' --Glacier.Router.Endpoints="default"' + \
          r' --Glacier.Router.Client.Endpoints="default"' + \
          r' --Glacier.Router.Server.Endpoints="tcp"'

print "starting glacier starter...",
starterPipe = os.popen(command, "r", 0)
TestUtil.getServerPid(starterPipe)
TestUtil.getAdapterReady(starterPipe)
print "ok"

name = os.path.join("Glacier", "starter")

TestUtil.mixedClientServerTest(name)

# We run the test again, to check whether the glacier router starter
# can start up multiple routers.
TestUtil.mixedClientServerTest(name)

print "shutting down glacier starter...",
TestUtil.killServers() # TODO: Graceful shutdown.
print "ok"

starterStatus = starterPipe.close()

if starterStatus:
    TestUtil.killServers()
    #sys.exit(1) # TODO: Uncomment when when we have graceful shutdown.

sys.exit(0)
