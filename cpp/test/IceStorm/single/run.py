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
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

name = os.path.join("IceStorm", "single")
testdir = os.path.join(toplevel, "test", name)

iceStorm = os.path.join(toplevel, "bin", "icestorm")
iceStormAdmin = os.path.join(toplevel, "bin", "icestormadmin")

updatedServerOptions = TestUtil.serverOptions.replace("TOPLEVELDIR", toplevel)
updatedClientOptions = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel)
updatedClientServerOptions = TestUtil.clientServerOptions.replace("TOPLEVELDIR", toplevel)

#
# Clean the contents of the database directory.
#
dbdir = os.path.join(testdir, "db")
TestUtil.cleanDbDir(dbdir)

iceStormEndpoint=' --IceStorm.TopicManager.Endpoints="default -p 12345"'

#
# TODO: --dbdir is a hack
#
command = iceStorm + updatedClientServerOptions + \
          iceStormEndpoint + \
          r' --dbdir ' + os.path.join(testdir, "db")

print "starting icestorm...",
iceStormPipe = os.popen(command)
TestUtil.getServerPid(iceStormPipe)
TestUtil.getAdapterReady(iceStormPipe)
print "ok"

command = iceStormAdmin + updatedClientOptions + \
          iceStormEndpoint + \
          r' -e "create single"'
print "creating topic...",
iceStormAdminPipe = os.popen(command)
iceStormAdminPipe.close()
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
except OSError:
    1 # Ignore

command = subscriber + updatedClientServerOptions + iceStormEndpoint + \
          r' ' + subscriberLockFile

print "starting subscriber...",
subscriberPipe = os.popen(command)
TestUtil.getServerPid(subscriberPipe)
TestUtil.getAdapterReady(subscriberPipe)
print "ok"

if not os.path.isfile(subscriberLockFile):
    print "subscriber lock file missing. failed!"
    TestUtil.killServers()
    sys.exit(1)

#
# Start the publisher. This should publish 10 events which eventually
# causes subscriber to terminate.
#
command = publisher + updatedClientOptions + iceStormEndpoint
print "starting publisher...",
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
        TestUtil.killServers()
        sys.exit(1)
    time.sleep(1)
    lockCount = lockCount + 1    

#
# Destroy the topic.
#
command = iceStormAdmin + updatedClientOptions + \
          iceStormEndpoint + \
          r' -e "destroy single"'
print "destroying topic...",
iceStormAdminPipe = os.popen(command)
iceStormAdminPipe.close()
print "ok"

#
# Shutdown icestorm.
#
command = iceStormAdmin + updatedClientOptions + \
          iceStormEndpoint + \
          r' -e "shutdown"'
print "shutting down icestorm...",
iceStormAdminPipe = os.popen(command)
iceStormAdminPipe.close()
print "ok"
