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
import time

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

if TestUtil.isWin32() and os.path.exists(os.path.join(toplevel, "bin", "iceboxd.exe")):
    iceBox = os.path.join(toplevel, "bin", "iceboxd")
else:
    iceBox = os.path.join(toplevel, "bin", "icebox")

iceBoxAdmin = os.path.join(toplevel, "bin", "iceboxadmin")
iceStormAdmin = os.path.join(toplevel, "bin", "icestormadmin")

iceBoxEndpoints = ' --IceBox.ServiceManager.Endpoints="default -p 12345" --Ice.Default.Locator='

iceStormService = " --IceBox.Service.IceStorm=IceStormService," + TestUtil.getIceSoVersion() + ":create" + \
                  ' --IceStorm.TopicManager.Endpoints="default -p 12346"' + \
                  ' --IceStorm.Publish.Endpoints="default"' + \
                  " --IceBox.PrintServicesReady=IceStorm"
iceStormReference = ' --IceStorm.TopicManager.Proxy="IceStorm/TopicManager: default -p 12346"'

def doTest(batch):
    global testdir
    global iceStormReference

    publisher = os.path.join(testdir, "publisher")
    subscriber = os.path.join(testdir, "subscriber")

    if batch:
        name = "batch subscriber"
        batchOptions = " -b"
    else:
        name = "subscriber"
        batchOptions = ""

    #
    # Start the subscriber. The subscriber creates a lock-file which
    # is used later to ensure that the subscriber actually goes away.
    #
    subscriberLockFile = os.path.join(testdir, 'subscriber.lock')
    try:
        os.remove(subscriberLockFile)
    except:
        pass # Ignore errors if the lockfile is not present

    print "starting " + name + "...",
    command = subscriber + batchOptions + TestUtil.clientServerOptions + iceStormReference + r' ' + subscriberLockFile + " 2>&1"
    subscriberPipe = os.popen(command)
    TestUtil.getServerPid(subscriberPipe)
    TestUtil.getAdapterReady(subscriberPipe)
    print "ok"

    print "checking " + name + " lockfile creation...",
    lockCount = 0
    while not os.path.isfile(subscriberLockFile):
        if lockCount > 10:
            print "failed!"
            TestUtil.killServers()
            sys.exit(1)
        time.sleep(1)
        lockCount = lockCount + 1    
    print "ok"

    #
    # Start the publisher. This should publish events which eventually
    # causes subscriber to terminate.
    #
    print "starting publisher...",
    command = publisher + TestUtil.clientOptions + iceStormReference + " 2>&1"
    publisherPipe = os.popen(command)
    print "ok"

    TestUtil.printOutputFromPipe(publisherPipe)

    #
    # Verify that the subscriber has terminated.
    #
    print "checking " + name + " lockfile removal...",
    lockCount = 0
    while os.path.isfile(subscriberLockFile):
        if lockCount > 60:
            print "failed!"
            TestUtil.killServers()
            sys.exit(1)
        time.sleep(1)
        lockCount = lockCount + 1    
    print "ok"

    subscriberStatus = subscriberPipe.close()
    publisherStatus = publisherPipe.close()

    return subscriberStatus or publisherStatus

dbHome = os.path.join(testdir, "db")
TestUtil.cleanDbDir(dbHome)
iceStormDBEnv=" --Freeze.DbEnv.IceStorm.DbHome=" + dbHome

print "starting icestorm service...",
command = iceBox + TestUtil.clientServerOptions + iceBoxEndpoints + iceStormService + iceStormDBEnv + " 2>&1"
iceBoxPipe = os.popen(command)
TestUtil.getServerPid(iceBoxPipe)
TestUtil.waitServiceReady(iceBoxPipe, "IceStorm")
print "ok"

print "creating topics...",
command = iceStormAdmin + TestUtil.clientOptions + iceStormReference + r' -e "create fed1 fed2 fed3"' + " 2>&1"
iceStormAdminPipe = os.popen(command)
iceStormAdminStatus = iceStormAdminPipe.close()
if iceStormAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
print "ok"

print "linking topics...",
command = iceStormAdmin + TestUtil.clientOptions + iceStormReference + r' -e "link fed1 fed2 10"' + " 2>&1"
iceStormAdminPipe = os.popen(command)
iceStormAdminStatus = iceStormAdminPipe.close()
if iceStormAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
command = iceStormAdmin + TestUtil.clientOptions + iceStormReference + r' -e "link fed2 fed3 5"' + " 2>&1"
iceStormAdminPipe = os.popen(command)
iceStormAdminStatus = iceStormAdminPipe.close()
if iceStormAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
print "ok"

#
# Test oneway subscribers.
#
onewayStatus = doTest(0)

#
# Test batch oneway subscribers.
#
batchStatus = doTest(1)

#
# Destroy the topics.
#
print "destroying topics...",
command = iceStormAdmin + TestUtil.clientOptions + iceStormReference + r' -e "destroy fed1 fed2 fed3"' + " 2>&1"
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
command = iceBoxAdmin + TestUtil.clientOptions + iceBoxEndpoints + r' shutdown' + " 2>&1"
iceBoxAdminPipe = os.popen(command)
iceBoxAdminStatus = iceBoxAdminPipe.close()
if iceBoxAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
print "ok"

iceBoxStatus = iceBoxPipe.close()

if iceBoxStatus or onewayStatus or batchStatus:
    TestUtil.killServers()
    sys.exit(1)

sys.exit(0)
