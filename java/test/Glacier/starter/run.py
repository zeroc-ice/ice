#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
# Mutable Realms, Inc.
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

if not os.environ.has_key('ICE_HOME'):
    print "ICE_HOME is not defined."
    sys.exit(0)

ice_home = os.environ['ICE_HOME']

starter = os.path.join(ice_home, "bin", "glacierstarter")
router = os.path.join(ice_home, "bin", "glacierrouter")

updatedServerOptions = TestUtil.serverOptions.replace("TOPLEVELDIR", toplevel)
updatedClientOptions = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel)
updatedClientServerOptions = TestUtil.clientServerOptions.replace("TOPLEVELDIR", toplevel)

command = starter + updatedClientServerOptions + \
          r' --Ice.PrintProcessId' \
          r' --Glacier.Starter.RouterPath=' + router + \
          r' --Glacier.Starter.PropertiesOverwrite=Ice.ServerIdleTime=10' \
          r' --Glacier.Starter.CryptPasswords="' + toplevel + r'/test/Glacier/starter/passwords"' + \
          r' --Glacier.Starter.Endpoints="default -p 12346 -t 5000"' + \
          r' --Glacier.Router.Endpoints="default"' + \
          r' --Glacier.Router.Client.Endpoints="default"' + \
          r' --Glacier.Router.Server.Endpoints="tcp"'

print "starting glacier starter...",
starterPipe = os.popen(command)
TestUtil.getServerPid(starterPipe)
TestUtil.getAdapterReady(starterPipe)
print "ok"

name = os.path.join("Glacier", "starter")
TestUtil.mixedClientServerTest(toplevel, name)

print "shutting down glacier starter...",
TestUtil.killServers() # TODO: Graceful shutdown
print "ok"

starterStatus = starterPipe.close()

# TODO: Enable again when we have graceful server starter shutdown.
#if starterStatus:
#    TestUtil.killServers()
#    sys.exit(1)

sys.exit(0)
