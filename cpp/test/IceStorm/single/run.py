#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys
import time

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil
TestUtil.processCmdLine()

name = os.path.join("IceStorm", "single")
testdir = os.path.dirname(os.path.abspath(__file__))


iceBox = TestUtil.getIceBox(testdir)
iceBoxAdmin = os.path.join(TestUtil.getBinDir(__file__), "iceboxadmin")
iceStormAdmin = os.path.join(TestUtil.getBinDir(__file__), "icestormadmin")

iceBoxEndpoints = ' --IceBox.ServiceManager.Endpoints="default -p 12010"'

iceStormService = " --IceBox.Service.IceStorm=IceStormService," + TestUtil.getIceSoVersion() + ":createIceStorm" + \
                  ' --IceStorm.TopicManager.Endpoints="default -p 12011"' + \
                  ' --IceStorm.Publish.Endpoints="default:udp"' + \
                  " --IceBox.PrintServicesReady=IceStorm" + \
                  " --IceBox.InheritProperties=1"
iceStormReference = ' --IceStorm.TopicManager.Proxy="IceStorm/TopicManager:default -p 12011"'

dbHome = os.path.join(testdir, "db")
TestUtil.cleanDbDir(dbHome)
iceStormDBEnv=" --Freeze.DbEnv.IceStorm.DbHome=" + dbHome

print "starting icestorm service...",
iceBoxPipe = TestUtil.startServer(iceBox, iceBoxEndpoints + iceStormService + iceStormDBEnv)
TestUtil.getServerPid(iceBoxPipe)
TestUtil.waitServiceReady(iceBoxPipe, "IceStorm")
print "ok"

print "creating topic...",
iceStormAdminPipe = TestUtil.startClient(iceStormAdmin, iceStormReference + r' -e "create single" 2>&1')
iceStormAdminStatus = TestUtil.closePipe(iceStormAdminPipe)
if iceStormAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
print "ok"

publisher = os.path.join(testdir, "publisher")
subscriber = os.path.join(testdir, "subscriber")

print "starting subscriber...",
subscriberPipe = TestUtil.startServer(subscriber, iceStormReference + " 2>&1")
TestUtil.getServerPid(subscriberPipe)
TestUtil.getAdapterReady(subscriberPipe, True, 5)
print "ok"

#
# Start the publisher. This should publish 10 events which eventually
# causes subscriber to terminate.
#
print "starting publisher...",
publisherPipe = TestUtil.startClient(publisher, iceStormReference + " 2>&1")
print "ok"

subscriberStatus = TestUtil.specificServerStatus(subscriberPipe, 30)
publisherStatus = TestUtil.closePipe(publisherPipe)

#
# Destroy the topic.
#
print "destroying topic...",
iceStormAdminPipe = TestUtil.startClient(iceStormAdmin, iceStormReference + r' -e "destroy single" 2>&1')
iceStormAdminStatus = TestUtil.closePipe(iceStormAdminPipe)
if iceStormAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
print "ok"

#
# Shutdown icestorm.
#
print "shutting down icestorm service...",
iceBoxAdminPipe = TestUtil.startClient(iceBoxAdmin, ' --IceBoxAdmin.ServiceManager.Proxy="IceBox/ServiceManager:default -p 12010"' + \
                                       r' shutdown  2>&1')
iceBoxAdminStatus = TestUtil.closePipe(iceBoxAdminPipe)
if iceBoxAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
print "ok"

if TestUtil.serverStatus() or subscriberStatus or publisherStatus:
    TestUtil.killServers()
    sys.exit(1)

sys.exit(0)
