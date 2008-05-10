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

name = os.path.join("IceStorm", "federation")
testdir = os.path.dirname(os.path.abspath(__file__))

import IceStormUtil

def doTest(icestorm, batch):

    publisher = os.path.join(testdir, "publisher")
    subscriber = os.path.join(testdir, "subscriber")

    if batch:
        name = "batch subscriber"
        batchOptions = " -b"
    else:
        name = "subscriber"
        batchOptions = ""

    subscriberPipe = TestUtil.startServer(subscriber, batchOptions + icestorm.reference() + " 2>&1")
    TestUtil.getServerPid(subscriberPipe)
    TestUtil.getAdapterReady(subscriberPipe)

    #
    # Start the publisher. This should publish events which eventually
    # causes subscriber to terminate.
    #
    publisherPipe = TestUtil.startClient(publisher, icestorm.reference() + " 2>&1")

    TestUtil.printOutputFromPipe(publisherPipe)

    subscriberStatus = TestUtil.specificServerStatus(subscriberPipe, 30)
    publisherStatus = TestUtil.closePipe(publisherPipe)

    return subscriberStatus or publisherStatus

def runtest(type, **args):
    icestorm = IceStormUtil.init(toplevel, testdir, type, **args)

    icestorm.start()

    print "setting up topics...",
    sys.stdout.flush()
    icestorm.admin("create fed1 fed2 fed3; link fed1 fed2 10; link fed2 fed3 5")
    print "ok"

    #
    # Test oneway subscribers.
    #
    print "testing oneway subscribers...",
    sys.stdout.flush()
    onewayStatus = doTest(icestorm, 0)
    print "ok"

    #
    # Test batch oneway subscribers.
    #
    print "testing batch subscribers...",
    sys.stdout.flush()
    batchStatus = doTest(icestorm, 1)
    print "ok"

    #
    # Destroy the topics.
    #
    print "destroying topics...",
    icestorm.admin("destroy fed1 fed2 fed3")
    print "ok"

    #
    # Shutdown icestorm.
    #
    icestorm.stop()

    if TestUtil.serverStatus() or onewayStatus or batchStatus:
	TestUtil.killServers()
	sys.exit(1)

runtest("persistent")
runtest("transient")
runtest("replicated", replicatedPublisher = False)
runtest("replicated", replicatedPublisher = True)

sys.exit(0)
