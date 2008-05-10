#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, time, re

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil
TestUtil.processCmdLine()
from threading import Thread

name = os.path.join("IceStorm", "repstress")
testdir = os.path.dirname(os.path.abspath(__file__))
publisher = os.path.join(testdir, "publisher")
subscriber = os.path.join(testdir, "subscriber")
control = os.path.join(testdir, "control")

def printOutput(pipe):
    try:
        while True:
            line = pipe.readline()
            if not line:
                break
            print line,
            sys.stdout.flush()
    except IOError:
        pass

def captureOutput(pipe):
    out = ""
    try:
        while True:
            line = pipe.readline()
            if not line:
                break
            out = out + line
    except IOError:
        pass
    return out

def runcontrol(proxy):
    pipe = TestUtil.startClient(control, ' "' + proxy + '"')
    printOutput(pipe)
    return TestUtil.closePipe(pipe)

import IceStormUtil

icestorm = IceStormUtil.init(toplevel, testdir, "replicated", replicatedPublisher=True, additional =
                             ' --IceStorm.Election.MasterTimeout=2' +
                             ' --IceStorm.Election.ElectionTimeout=2' +
                             ' --IceStorm.Election.ResponseTimeout=2')
icestorm.start()

print "creating topic...",
sys.stdout.flush()
icestorm.admin("create single")
print "ok"

print "running subscriber...",
sys.stdout.flush()
subscriberPipe = TestUtil.startServer(subscriber, ' --Ice.ServerIdleTime=0 ' + icestorm.reference())
TestUtil.getServerPid(subscriberPipe)
TestUtil.getAdapterReady(subscriberPipe, False)
subControl = subscriberPipe.readline().strip()
print "ok"

print "running publisher...",
sys.stdout.flush()
publisherPipe = TestUtil.startServer(publisher, ' --Ice.ServerIdleTime=0 ' + icestorm.reference())
TestUtil.getServerPid(publisherPipe)
TestUtil.getAdapterReady(publisherPipe, False)
pubControl = publisherPipe.readline().strip()
print "ok"

time.sleep(2)

for i in range(0, 3):
    # 0, 1
    print "stopping replica 2 (0, 1 running)...",
    sys.stdout.flush()
    icestorm.stopReplica(2)
    print "ok"
    time.sleep(2)

    # 1, 2
    print "starting 2, stopping 0 (1, 2 running)...",
    sys.stdout.flush()
    icestorm.startReplica(2, echo=False)
    icestorm.stopReplica(0)
    print "ok"
    # This waits for the replication to startup
    #icestorm.admin("list")
    time.sleep(2)

    # 0, 2
    print "starting 0, stopping 1 (0, 2 running)...",
    sys.stdout.flush()
    icestorm.startReplica(0, echo=False)
    icestorm.stopReplica(1)
    print "ok"
    # This waits for the replication to startup
    #icestorm.admin("list")
    time.sleep(2)

    print "starting 1 (all running)...",
    sys.stdout.flush()
    icestorm.startReplica(1, echo=False)
    print "ok"
    # This waits for the replication to startup
    #icestorm.admin("list")

    time.sleep(2)

print "stopping publisher...",
sys.stdout.flush()
if runcontrol(pubControl):
    printOutput(publisherPipe)
    TestUtil.killServers()
    sys.exit(1)
publisherCount = publisherPipe.readline().strip()
if TestUtil.closePipe(publisherPipe):
    TestUtil.killServers()
    sys.exit(1)
print "ok"

print "stopping replicas...",
sys.stdout.flush()
icestorm.stop()
print "ok"

print "stopping subscriber...",
sys.stdout.flush()
if runcontrol(subControl):
    printOutput(subscriberPipe)
    TestUtil.killServers()
    sys.exit(1)
subscriberCount = subscriberPipe.readline().strip()
if TestUtil.closePipe(subscriberPipe):
    TestUtil.killServers()
    sys.exit(1)
print "ok"

print "publisher published %s events, subscriber received %s events" % (publisherCount, subscriberCount)
#print "comparing counts...",
#sys.stdout.flush()
#if publisherCount != subscriberCount:
    #TestUtil.killServers()
    #sys.exit(1)
#print "ok"

if TestUtil.serverStatus():
    TestUtil.killServers()
    sys.exit(1)

sys.exit(0)
