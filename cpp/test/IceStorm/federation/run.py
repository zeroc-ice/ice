#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
#
# **********************************************************************

import os, sys, time

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

name = os.path.join("IceStorm", "federation")
testdir = os.path.join(toplevel, "test", name)

iceBox = os.path.join(toplevel, "bin", "icebox")
iceBoxAdmin = os.path.join(toplevel, "bin", "iceboxadmin")
iceStormAdmin = os.path.join(toplevel, "bin", "icestormadmin")

iceBoxEndpoints = ' --IceBox.ServiceManager.Endpoints="default -p 12345"'

iceStormService = " --IceBox.Service.IceStorm=IceStormService:create" + \
                  ' --IceStorm.TopicManager.Endpoints="default -p 12346"' + \
                  " --IceBox.PrintServicesReady=IceStorm"
iceStormReference = ' --IceStorm.TopicManager.Proxy="IceStorm/TopicManager: default -p 12346"'

dbEnvName = os.path.join(testdir, "db")
TestUtil.cleanDbDir(dbEnvName)
iceStormDBEnv=" --IceBox.DBEnvName.IceStorm=" + dbEnvName

print "starting icestorm service...",
command = iceBox + TestUtil.clientServerOptions + iceBoxEndpoints + iceStormService + iceStormDBEnv
iceBoxPipe = os.popen(command)
TestUtil.getServerPid(iceBoxPipe)
TestUtil.waitServiceReady(iceBoxPipe, "IceStorm")
print "ok"

print "creating topics...",
command = iceStormAdmin + TestUtil.clientOptions + iceStormReference + r' -e "create fed1"'
iceStormAdminPipe = os.popen(command)
iceStormAdminStatus = iceStormAdminPipe.close()
if iceStormAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
command = iceStormAdmin + TestUtil.clientOptions + iceStormReference + r' -e "create fed2"'
iceStormAdminPipe = os.popen(command)
iceStormAdminStatus = iceStormAdminPipe.close()
if iceStormAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
command = iceStormAdmin + TestUtil.clientOptions + iceStormReference + r' -e "create fed3"'
iceStormAdminPipe = os.popen(command)
iceStormAdminStatus = iceStormAdminPipe.close()
if iceStormAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
print "ok"

print "linking topics...",
command = iceStormAdmin + TestUtil.clientOptions + iceStormReference + r' -e "link fed1 fed2 10"'
iceStormAdminPipe = os.popen(command)
iceStormAdminStatus = iceStormAdminPipe.close()
if iceStormAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
command = iceStormAdmin + TestUtil.clientOptions + iceStormReference + r' -e "link fed2 fed3 5"'
iceStormAdminPipe = os.popen(command)
iceStormAdminStatus = iceStormAdminPipe.close()
if iceStormAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
print "ok"

publisher = os.path.join(testdir, "publisher")
subscriber = os.path.join(testdir, "subscriber")

#
# Start the subscriber. The subscriber creates a lock-file which
# is used later to ensure that the subscriber actually goes away.
#
subscriberLockFile = os.path.join(testdir, 'subscriber.lock')
try:
    os.remove(subscriberLockFile)
except:
    pass # Ignore errors if the lockfile is not present

print "starting subscriber...",
command = subscriber + TestUtil.clientServerOptions + iceStormReference + r' ' + subscriberLockFile
subscriberPipe = os.popen(command)
TestUtil.getServerPid(subscriberPipe)
TestUtil.getAdapterReady(subscriberPipe)
print "ok"

print "checking subscriber lockfile creation...",
if not os.path.isfile(subscriberLockFile):
    print "failed!"
    TestUtil.killServers()
    sys.exit(1)
print "ok"

#
# Start the publisher. This should publish events which eventually
# causes subscriber to terminate.
#
print "starting publisher...",
command = publisher + TestUtil.clientOptions + iceStormReference
publisherPipe = os.popen(command)
print "ok"

for output in publisherPipe.xreadlines():
    print output,

#
# Verify that the subscriber has terminated.
#
print "checking subscriber lockfile removal...",
lockCount = 0
while os.path.isfile(subscriberLockFile):
    if lockCount > 10:
        print "failed!"
        TestUtil.killServers()
        sys.exit(1)
    time.sleep(1)
    lockCount = lockCount + 1    
print "ok"

#
# Destroy the topic.
#
print "destroying topics...",
command = iceStormAdmin + TestUtil.clientOptions + iceStormReference + r' -e "destroy fed1"'
iceStormAdminPipe = os.popen(command)
iceStormAdminStatus = iceStormAdminPipe.close()
if iceStormAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
command = iceStormAdmin + TestUtil.clientOptions + iceStormReference + r' -e "destroy fed2"'
iceStormAdminPipe = os.popen(command)
iceStormAdminStatus = iceStormAdminPipe.close()
if iceStormAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
command = iceStormAdmin + TestUtil.clientOptions + iceStormReference + r' -e "destroy fed3"'
iceStormAdminPipe = os.popen(command)
iceStormAdminStatus = iceStormAdminPipe.close()
if iceStormAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
print "ok"

#
# Shutdown icestorm.
#
print "shutting down icestorm service...",
command = iceBoxAdmin + TestUtil.clientOptions + iceBoxEndpoints + r' shutdown'
iceBoxAdminPipe = os.popen(command)
iceBoxAdminStatus = iceBoxAdminPipe.close()
if iceBoxAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
print "ok"

iceBoxStatus = iceBoxPipe.close()
subscriberStatus = subscriberPipe.close()
publisherStatus = publisherPipe.close()

if iceBoxStatus or subscriberStatus or publisherStatus:
    TestUtil.killServers()
    sys.exit(1)

sys.exit(0)
