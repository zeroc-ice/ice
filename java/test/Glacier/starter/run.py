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

#
# SSL isn't supported in Java
#
#if TestUtil.protocol != "ssl":
#    print "This test may only be run using the SSL protocol."
#    sys.exit(0)

if not os.environ.has_key('ICE_HOME'):
    print "ICE_HOME is not defined."
    sys.exit(0)

ice_home = os.environ['ICE_HOME']

starter = os.path.join(ice_home, "bin", "glacierstarter")
router = os.path.join(ice_home, "bin", "glacier")

updatedServerOptions = TestUtil.serverOptions.replace("TOPLEVELDIR", toplevel)
updatedClientOptions = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel)
updatedClientServerOptions = TestUtil.clientServerOptions.replace("TOPLEVELDIR", toplevel)

command = starter + updatedClientServerOptions + \
          r' --Ice.PrintProcessId' \
          r' --Glacier.Starter.RouterPath=' + router + \
          r' --Glacier.Starter.Endpoints="default -p 12346 -t 5000"' + \
          r' --Glacier.Router.Endpoints="default"' + \
          r' --Glacier.Client.Endpoints="default"' + \
          r' --Glacier.Server.Endpoints="tcp"' + \
          r' --Glacier.Starter.Certificate.Country=US' + \
          r' --Glacier.Starter.Certificate.StateProvince=Alabama' + \
          r' --Glacier.Starter.Certificate.Locality=Huntsville' + \
          r' --Glacier.Starter.Certificate.Organization="Mutable Realms"' + \
          r' --Glacier.Starter.Certificate.OrganizationalUnit="Ice Age Game"' + \
          r' --Glacier.Starter.Certificate.CommonName="Ice Age Certificate"' + \
          r' --Glacier.Starter.Certificate.BitStrength=1024' + \
          r' --Glacier.Starter.Certificate.SecondsValid=31536000'

print "starting glacier starter...",
starterPipe = os.popen(command)
TestUtil.getServerPid(starterPipe)
TestUtil.getAdapterReady(starterPipe)
print "ok"

name = os.path.join("Glacier", "starter")
TestUtil.clientServerHybridTest(toplevel, name)

print "shutting down glacier starter...",
TestUtil.killServers() # TODO: Graceful shutdown
print "ok"

sys.exit(0)
