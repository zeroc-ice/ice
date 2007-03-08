#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, time, threading, re

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

name = os.path.join("IceStorm", "federation2")
testdir = os.path.join(toplevel, "test", name)

exedir = os.path.join(toplevel, "test", "IceStorm", "federation2")

iceBox = TestUtil.getIceBox(exedir)
iceBoxAdmin = os.path.join(toplevel, "bin", "iceboxadmin")
iceStormAdmin = os.path.join(toplevel, "bin", "icestormadmin")

iceBoxEndpoints = ' --IceBox.ServiceManager.Endpoints="default -p 12010" --Ice.Default.Locator='

iceStormService = " --IceBox.Service.IceStorm=IceStormService," + TestUtil.getIceSoVersion() + ":createIceStorm" + \
                  ' --IceStorm.TopicManager.Endpoints="default -p 12011"' + \
                  ' --IceStorm.Publish.Endpoints="default -p 12012"' + \
                  ' --IceStorm.InstanceName=TestIceStorm1 ' + \
                  ' --IceStorm.Discard.Interval=2' + \
                  ' --IceBox.PrintServicesReady=IceStorm' + \
                  " --IceBox.InheritProperties=1"
iceStormReference = ' --IceStorm.TopicManager.Proxy="TestIceStorm1/TopicManager: default -p 12011"'

iceBoxEndpoints2 = ' --IceBox.ServiceManager.Endpoints="default -p 12020" --Ice.Default.Locator='

iceStormService2 = " --IceBox.Service.IceStorm=IceStormService," + TestUtil.getIceSoVersion() + ":createIceStorm" + \
                  ' --IceStorm.TopicManager.Endpoints="default -p 12021"' + \
                  ' --IceStorm.Publish.Endpoints="default -p 12022"' + \
                  ' --IceStorm.InstanceName=TestIceStorm2 ' + \
                  ' --IceStorm.Discard.Interval=2' + \
                  ' --IceBox.PrintServicesReady=IceStorm' + \
                  " --IceBox.InheritProperties=1"
iceStormReference2 = ' --IceStorm.TopicManager.Proxy="TestIceStorm2/TopicManager: default -p 12021"'

adminIceStormReference = ' --IceStormAdmin.TopicManager.Proxy="TestIceStorm1/TopicManager: default -p 12011" ' + \
    '--IceStormAdmin.TopicManager.Proxy2="TestIceStorm2/TopicManager: default -p 12021"'

def doTest(batch, subscriberRef = None):
    global testdir
    global iceStormReference
    global iceStormReference2

    publisher = os.path.join(toplevel, "test", "IceStorm", "federation2", "publisher")
    subscriber = os.path.join(toplevel, "test", "IceStorm", "federation2", "subscriber")

    if batch:
        name = "batch subscriber"
        batchOptions = " -b"
    else:
        name = "subscriber"
        batchOptions = ""

    if subscriberRef == None:
        subscriberRef = iceStormReference2

    command = subscriber + batchOptions + TestUtil.clientServerOptions + subscriberRef
    if TestUtil.debug:
        print "(" + command + ")",
    subscriberPipe = os.popen(command + " 2>&1")
    TestUtil.getServerPid(subscriberPipe)
    TestUtil.getAdapterReady(subscriberPipe)

    #
    # Start the publisher. This should publish events which eventually
    # causes subscriber to terminate.
    #
    command = publisher + TestUtil.clientOptions + iceStormReference
    if TestUtil.debug:
        print "(" + command + ")",
    publisherPipe = os.popen(command + " 2>&1")

    TestUtil.printOutputFromPipe(publisherPipe)

    subscriberStatus = TestUtil.specificServerStatus(subscriberPipe, 30)
    publisherStatus = TestUtil.closePipe(publisherPipe)

    return subscriberStatus or publisherStatus

def startServers():
    global iceBox
    global iceBoxEndpoints
    global iceBoxEndpoints2
    global iceStormService
    global iceStormService2
    global iceStormDBEnv
    global iceStormDBEnv2
    command = iceBox + TestUtil.clientServerOptions + iceBoxEndpoints + iceStormService + iceStormDBEnv
    if TestUtil.debug:
        print "(" + command + ")",
    iceBoxPipe = os.popen(command + " 2>&1")
    TestUtil.getServerPid(iceBoxPipe)
    TestUtil.waitServiceReady(iceBoxPipe, "IceStorm")
    command = iceBox + TestUtil.clientServerOptions + iceBoxEndpoints2 + iceStormService2 + iceStormDBEnv2
    if TestUtil.debug:
        print "(" + command + ")",
    iceBoxPipe2 = os.popen(command + " 2>&1")
    TestUtil.getServerPid(iceBoxPipe2)
    TestUtil.waitServiceReady(iceBoxPipe2, "IceStorm")

    return iceBoxPipe, iceBoxPipe2

def stopServers(p1, p2 = None):
    global iceBox
    global iceBoxAdmin
    global iceBoxEndpoints
    global iceBoxEndpoints2
    command = iceBoxAdmin + TestUtil.clientOptions + iceBoxEndpoints + r' shutdown'
    if TestUtil.debug:
        print "(" + command + ")",
    pipe = os.popen(command + " 2>&1")
    status = TestUtil.closePipe(pipe)
    if status or TestUtil.specificServerStatus(p1):
        TestUtil.killServers()
        sys.exit(1)
    if p2:
        command = iceBoxAdmin + TestUtil.clientOptions + iceBoxEndpoints2 + r' shutdown'
        if TestUtil.debug:
            print "(" + command + ")",
        pipe = os.popen(command + " 2>&1")
        status = TestUtil.closePipe(pipe)
        if status or TestUtil.specificServerStatus(p2):
            TestUtil.killServers()
            sys.exit(1)

dbHome = os.path.join(testdir, "db")
TestUtil.cleanDbDir(dbHome)
iceStormDBEnv=" --Freeze.DbEnv.IceStorm.DbHome=" + dbHome

dbHome2 = os.path.join(testdir, "db2")
TestUtil.cleanDbDir(dbHome2)
iceStormDBEnv2=" --Freeze.DbEnv.IceStorm.DbHome=" + dbHome2

#
# Test #1:
#
# create a cross service link fed1->fed2 and ensure the events are
# published between them correctly.
#
print "starting IceStorm services...",
sys.stdout.flush()
iceBoxPipe1, iceBoxPipe2 = startServers()
print "ok"

print "setting up the topics...",
sys.stdout.flush()
command = iceStormAdmin + TestUtil.clientOptions + adminIceStormReference + \
    r' -e "create TestIceStorm1/fed1 TestIceStorm2/fed1; link TestIceStorm1/fed1 TestIceStorm2/fed1"'
if TestUtil.debug:
    print "(" + command + ")",
iceStormAdminPipe = os.popen(command + " 2>&1")
iceStormAdminStatus = TestUtil.closePipe(iceStormAdminPipe)
if iceStormAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
print "ok"

#
# Test oneway subscribers.
#
print "testing federation with oneway subscribers...",
sys.stdout.flush()
onewayStatus = doTest(0)
print "ok"

#
# Test batch oneway subscribers.
#
print "testing federation with batch subscribers...",
sys.stdout.flush()
batchStatus = doTest(1)
print "ok"

if onewayStatus or batchStatus:
    TestUtil.killServers()
    sys.exit(1)

#
# Test #2:
#
# Stop and restart the service and repeat the test. This ensures that
# the database is correct.
#
print "restarting services to ensure that the database content is preserved...",
sys.stdout.flush()

#
# Shutdown icestorm.
#
stopServers(iceBoxPipe1, iceBoxPipe2)

iceBoxPipe1, iceBoxPipe2 = startServers()
print "ok"

#
# Test oneway subscribers.
#
print "retesting federation with oneway subscribers... ",
sys.stdout.flush()
onewayStatus = doTest(0)
print "ok"

#
# Test batch oneway subscribers.
#
print "retesting federation with batch subscribers... ",
sys.stdout.flush()
batchStatus = doTest(1)
print "ok"

if onewayStatus or batchStatus:
    TestUtil.killServers()
    sys.exit(1)

#
# Shutdown icestorm.
#
stopServers(iceBoxPipe1, iceBoxPipe2)

#
# This is used by the below test to confirm that the link warning is
# emitted. This class conforms with the TestUtil.ReaderThread protocol.
#
class ExpectorThread(threading.Thread):
    def __init__(self, pipe):
        self.mutex = threading.Lock()
        self.pipe = pipe
        # Suppress "adapter ready" messages. Under windows the eol isn't \n.
        self.re = [ [ re.compile(" ready\r?\n$"), 0 ] ]
        threading.Thread.__init__(self)

    def run(self):
        try:
            while 1:
                line = self.pipe.readline()
                if not line: break
                found = False;
                self.mutex.acquire()
                for item in self.re:
                    if item[0].search(line):
                        found = True
                        item[1] = item[1] + 1
                        break
                self.mutex.release()
                if not found:
                    print line,
        except IOError:
            pass

        self.status = TestUtil.closePipe(self.pipe)

    # To comply with the ReaderThread protocol.
    def getPipe(self):
        return self.pipe

    # To comply with the ReaderThread protocol.
    def getStatus(self):
        return self.status

    def matches(self, index):
        self.mutex.acquire()
        m = self.re[index][1]
        self.mutex.release()
        return m

    def expect(self, r):
        self.mutex.acquire()
        self.re.append([r, 0])
        l = len(self.re)-1
        self.mutex.release()
        return l

#
# Test #3:
#
# Restart the first server and publish some events. Attach a
# subscriber to the channel and make sure the events are received.
#
# Then re-start the linked downstream server and publish the events.
# Ensure they are received by the linked server.
#
print "restarting only one IceStorm server...",
sys.stdout.flush()
command = iceBox + TestUtil.clientServerOptions + iceBoxEndpoints + iceStormService + iceStormDBEnv
if TestUtil.debug:
    print "(" + command + ")",
iceBoxPipe1 = os.popen(command + " 2>&1")
TestUtil.getServerPid(iceBoxPipe1)
TestUtil.waitServiceReady(iceBoxPipe1, "IceStorm", False)
expectorThread = ExpectorThread(iceBoxPipe1)
expectorThread.start()
global serverThreads
TestUtil.serverThreads.append(expectorThread)
index = expectorThread.expect(re.compile("fed1.link.*link offline"))
expectorThread.expect(re.compile("connection refused"))
print "ok"

#
# Test oneway subscribers.
#
print "testing that the federation link reports an error...",
sys.stdout.flush()
onewayStatus = doTest(0, iceStormReference)
print "ok"

if onewayStatus or expectorThread.matches(index) != 1:
    TestUtil.killServers()
    sys.exit(1)

print "starting downstream icestorm server...",
sys.stdout.flush()
command = iceBox + TestUtil.clientServerOptions + iceBoxEndpoints2 + iceStormService2 + iceStormDBEnv2
if TestUtil.debug:
    print "(" + command + ")",
iceBoxPipe2 = os.popen(command + " 2>&1")
TestUtil.getServerPid(iceBoxPipe2)
TestUtil.waitServiceReady(iceBoxPipe2, "IceStorm")
print "ok"

#
# Need to sleep for at least the discard interval.
#
time.sleep(3)

#
# Test oneway subscribers.
#
print "testing link is reestablished...",
sys.stdout.flush()
onewayStatus = doTest(0)
print "ok"

if onewayStatus or expectorThread.matches(index) != 1:
    TestUtil.killServers()
    sys.exit(1)

#
# Test #4:
#
# Trash the TestIceStorm2 database. Then restart the servers and
# verify that the link is removed.
#
print "destroying the downstream IceStorm service database...",
sys.stdout.flush()
stopServers(iceBoxPipe1, iceBoxPipe2)

TestUtil.cleanDbDir(dbHome2)
print "ok"

print "restarting IceStorm servers...",
sys.stdout.flush()
iceBoxPipe1, iceBoxPipe2 = startServers()
print "ok"

print "checking link still exists...",
command = iceStormAdmin + TestUtil.clientOptions + adminIceStormReference + r' -e "links TestIceStorm1"'
if TestUtil.debug:
    print "(" + command + ")",
iceStormAdminPipe = os.popen(command + " 2>&1")
line = iceStormAdminPipe.readline()
if not re.compile("fed1 with cost 0").search(line):
    print line
    TestUtil.killServers()
    sys.exit(1)
iceStormAdminStatus = TestUtil.closePipe(iceStormAdminPipe)
if iceStormAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
print "ok"

print "publishing some events...",
sys.stdout.flush()
publisher = os.path.join(toplevel, "test", "IceStorm", "federation2", "publisher")
command = publisher + TestUtil.clientOptions + iceStormReference
if TestUtil.debug:
    print "(" + command + ")",
publisherPipe = os.popen(command + " 2>&1")

TestUtil.printOutputFromPipe(publisherPipe)

publisherStatus = TestUtil.closePipe(publisherPipe)
if publisherStatus:
    TestUtil.killServers()
    sys.exit(1)

# The publisher must be run twice because all the events can be sent
# out in one batch to the linked subscriber which means that the link
# is not reaped until the next batch is sent.
time.sleep(1)
publisher = os.path.join(toplevel, "test", "IceStorm", "federation2", "publisher")
command = publisher + TestUtil.clientOptions + iceStormReference
if TestUtil.debug:
    print "(" + command + ")",
publisherPipe = os.popen(command + " 2>&1")
print "ok"

TestUtil.printOutputFromPipe(publisherPipe)

publisherStatus = TestUtil.closePipe(publisherPipe)
if publisherStatus:
    TestUtil.killServers()
    sys.exit(1)

# Verify that the link has disappeared.
print "verifying that the link has been destroyed...",
sys.stdout.flush()
command = iceStormAdmin + TestUtil.clientOptions + adminIceStormReference + \
    r' -e "links TestIceStorm1"' + " 2>&1"
if TestUtil.debug:
    print "(" + command + ")",
iceStormAdminPipe = os.popen(command + " 2>&1")
line = iceStormAdminPipe.readline()
try:
    if line and len(line) > 0:
        TestUtil.killServers()
        sys.exit(1)
except IOError:
    pass
iceStormAdminStatus = TestUtil.closePipe(iceStormAdminPipe)
if iceStormAdminStatus:
    print iceStormAdminStatus
    TestUtil.killServers()
    sys.exit(1)
print "ok"

#
# Destroy the remaining topic.
#
print "destroying topics...",
command = iceStormAdmin + TestUtil.clientOptions + adminIceStormReference + \
    r' -e "destroy TestIceStorm1/fed1"' + " 2>&1"
if TestUtil.debug:
    print "(" + command + ")",
iceStormAdminPipe = os.popen(command + " 2>&1")
iceStormAdminStatus = TestUtil.closePipe(iceStormAdminPipe)
if iceStormAdminStatus:
    TestUtil.killServers()
    sys.exit(1)
print "ok"

#
# Shutdown icestorm.
#
print "shutting down icestorm services...",
sys.stdout.flush()
stopServers(iceBoxPipe1, iceBoxPipe2)
if TestUtil.serverStatus():
    TestUtil.killServers()
    sys.exit(1)
print "ok"

sys.exit(0)
