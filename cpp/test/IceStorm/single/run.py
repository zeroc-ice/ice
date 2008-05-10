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

import IceStormUtil

def dotest(type):
    icestorm = IceStormUtil.init(toplevel, testdir, type)

    icestorm.start()

    print "creating topic...",
    sys.stdout.flush()
    icestorm.admin("create single")
    print "ok"

    publisher = os.path.join(testdir, "publisher")
    subscriber = os.path.join(testdir, "subscriber")

    print "starting subscriber...",
    sys.stdout.flush()
    subscriberPipe = TestUtil.startServer(subscriber, icestorm.reference())
    TestUtil.getServerPid(subscriberPipe)
    TestUtil.getAdapterReady(subscriberPipe, True, 5)
    print "ok"

    #
    # Start the publisher. This should publish 10 events which eventually
    # causes subscriber to terminate.
    #
    print "starting publisher...",
    sys.stdout.flush()
    publisherPipe = TestUtil.startClient(publisher, icestorm.reference())
    print "ok"

    subscriberStatus = TestUtil.specificServerStatus(subscriberPipe, 30)
    publisherStatus = TestUtil.closePipe(publisherPipe)

    #
    # Destroy the topic.
    #
    print "destroy topic...",
    sys.stdout.flush()
    icestorm.admin("destroy single")
    print "ok"

    #
    # Shutdown icestorm.
    #
    icestorm.stop()

    if TestUtil.serverStatus() or subscriberStatus or publisherStatus:
        TestUtil.killServers()
        sys.exit(1)

dotest("persistent")
dotest("transient")
dotest("replicated")

sys.exit(0)
