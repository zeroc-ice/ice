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

import os, sys, time

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

testdir = os.path.join(toplevel, "test", "IceStorm", "federation2")
exedir = os.path.join(toplevel, "test", "IceStorm", "federation")

iceStorm = os.path.join(toplevel, "bin", "icestorm")
iceStormAdmin = os.path.join(toplevel, "bin", "icestormadmin")

updatedServerOptions = TestUtil.serverOptions.replace("TOPLEVELDIR", toplevel)
updatedClientOptions = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel)
updatedClientServerOptions = TestUtil.clientServerOptions.replace("TOPLEVELDIR", toplevel)

iceStormEndpoint=' --IceStorm.TopicManager.Endpoints="default -p 12345"'

print "starting icestorm...",
dbEnvName = os.path.join(testdir, "db")
TestUtil.cleanDbDir(dbEnvName)
command = iceStorm + updatedClientServerOptions + iceStormEndpoint + " --IceStorm.DBEnvName=" + dbEnvName
iceStormPipe = os.popen(command)
TestUtil.getServerPid(iceStormPipe)
TestUtil.getAdapterReady(iceStormPipe)
print "ok"

print "creating topics...",
command = iceStormAdmin + updatedClientOptions + iceStormEndpoint + r' -e "create fed1"'
iceStormAdminPipe = os.popen(command)
iceStormAdminPipe.close()
command = iceStormAdmin + updatedClientOptions + iceStormEndpoint + r' -e "create fed2"'
iceStormAdminPipe = os.popen(command)
iceStormAdminPipe.close()
command = iceStormAdmin + updatedClientOptions + iceStormEndpoint + r' -e "create fed3"'
iceStormAdminPipe = os.popen(command)
iceStormAdminPipe.close()
print "ok"

print "linking topics...",
graph = os.path.join(testdir, "graph.xml");
command = iceStormAdmin + updatedClientOptions + iceStormEndpoint + r' -e "graph \"' + graph + r'\" 10"'
iceStormAdminPipe = os.popen(command)
iceStormAdminPipe.close()
print "ok"

publisher = os.path.join(exedir, "publisher")
subscriber = os.path.join(exedir, "subscriber")

#
# Start the subscriber. The subscriber creates a lock-file which
# is used later to ensure that the subscriber actually goes away.
#
subscriberLockFile = os.path.join(testdir, 'subscriber.lock')
try:
    os.remove(subscriberLockFile)
except:
    pass # Ignore errors if the lock file is not present

print "starting subscriber...",
command = subscriber + updatedClientServerOptions + iceStormEndpoint + r' ' + subscriberLockFile
subscriberPipe = os.popen(command)
TestUtil.getServerPid(subscriberPipe)
TestUtil.getAdapterReady(subscriberPipe)
print "ok"

if not os.path.isfile(subscriberLockFile):
    print "subscriber lock file missing. failed!"
    TestUtil.killServers()
    sys.exit(1)

#
# Start the publisher. This should publish events which eventually
# causes subscriber to terminate.
#
print "starting publisher...",
command = publisher + updatedClientOptions + iceStormEndpoint
publisherPipe = os.popen(command)
output = publisherPipe.readline()
if not output:
    print "failed!"
    TestUtil.killServers()
    sys.exit(1)
print "ok"

#
# Verify that the subscriber has terminated.
#
lockCount = 0
while os.path.isfile(subscriberLockFile):
    #
    # TODO: debugging
    #
    print "lock file still present: ", lockCount
    if lockCount > 10:
        print "subscriber didn't terminate, failed!"
#        TestUtil.killServers()
        sys.exit(1)
    time.sleep(1)
    lockCount = lockCount + 1    

#
# Destroy the topic.
#
print "destroying topics...",
command = iceStormAdmin + updatedClientOptions + iceStormEndpoint + r' -e "destroy fed1"'
iceStormAdminPipe = os.popen(command)
iceStormAdminPipe.close()
command = iceStormAdmin + updatedClientOptions + iceStormEndpoint + r' -e "destroy fed2"'
iceStormAdminPipe = os.popen(command)
iceStormAdminPipe.close()
command = iceStormAdmin + updatedClientOptions + iceStormEndpoint + r' -e "destroy fed3"'
iceStormAdminPipe = os.popen(command)
iceStormAdminPipe.close()
print "ok"

#
# Shutdown icestorm.
#
print "shutting down icestorm...",
command = iceStormAdmin + updatedClientOptions + iceStormEndpoint + r' -e "shutdown"'
iceStormAdminPipe = os.popen(command)
iceStormAdminPipe.close()
print "ok"
