#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys
import time

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil, IceStormUtil

publisher = os.path.join(os.getcwd(), "publisher")
subscriber = os.path.join(os.getcwd(), "subscriber")

targets = []
if TestUtil.appverifier:
    targets = [TestUtil.getIceBox(), publisher, subscriber, TestUtil.getIceBoxAdmin(), TestUtil.getIceStormAdmin()]
    TestUtil.setAppVerifierSettings(targets, cwd = os.getcwd())

def doTest(icestorm, batch):

    if batch:
        name = "batch subscriber"
        batchOptions = " -b"
    else:
        name = "subscriber"
        batchOptions = ""

    subscriberProc = TestUtil.startServer(subscriber, batchOptions + icestorm.reference())

    #
    # Start the publisher. This should publish events which eventually
    # causes subscriber to terminate.
    #
    publisherProc = TestUtil.startClient(publisher, icestorm.reference())
    subscriberProc.waitTestSuccess()
    publisherProc.waitTestSuccess()

def runtest(type, **args):
    icestorm = IceStormUtil.init(TestUtil.toplevel, os.getcwd(), type, **args)

    icestorm.start()

    sys.stdout.write("setting up topics... ")
    sys.stdout.flush()
    icestorm.admin("create fed1 fed2 fed3; link fed1 fed2 10; link fed2 fed3 5")
    print("ok")

    #
    # Test oneway subscribers.
    #
    sys.stdout.write("testing oneway subscribers... ")
    sys.stdout.flush()
    doTest(icestorm, 0)
    print("ok")

    #
    # Test batch oneway subscribers.
    #
    sys.stdout.write("testing batch subscribers... ")
    sys.stdout.flush()
    doTest(icestorm, 1)
    print("ok")

    #
    # Destroy the topics.
    #
    sys.stdout.write("destroying topics... ")
    sys.stdout.flush()
    icestorm.admin("destroy fed1 fed2 fed3")
    print("ok")

    #
    # Shutdown icestorm.
    #
    icestorm.stop()

runtest("persistent")
runtest("transient")
runtest("replicated", replicatedPublisher = False)
runtest("replicated", replicatedPublisher = True)

if TestUtil.appverifier:
    TestUtil.appVerifierAfterTestEnd(targets, cwd = os.getcwd())
