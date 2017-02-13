#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

#
# Make sure IceStorm and the subscriber use the same buffer size for
# sending/receiving datagrams. This ensures the test works with bogus
# OS configurations where the reicever buffer size is smaller than the
# send buffer size (causing the received messages to be
# truncated). See also bug #6070.
#
iceStormArgs = " --Ice.UDP.SndSize=4096"
subscriberArgs = " --Ice.UDP.RcvSize=4096"

targets = []
if TestUtil.appverifier:
    targets = [TestUtil.getIceBox(), publisher, subscriber, TestUtil.getIceBoxAdmin(), TestUtil.getIceStormAdmin()]
    TestUtil.setAppVerifierSettings(targets, cwd = os.getcwd())

def dotest(type):
    icestorm = IceStormUtil.init(TestUtil.toplevel, os.getcwd(), type, additional=iceStormArgs)
    icestorm.start()

    sys.stdout.write("creating topic... ")
    sys.stdout.flush()
    icestorm.admin("create single")
    print("ok")

    sys.stdout.write("starting subscriber... ")
    sys.stdout.flush()
    subscriberProc = TestUtil.startServer(subscriber, icestorm.reference() + subscriberArgs, count = 3)
    print("ok")

    #
    # Start the publisher. This should publish 10 events which eventually
    # causes subscriber to terminate.
    #
    sys.stdout.write("starting publisher... ")
    sys.stdout.flush()
    publisherProc = TestUtil.startClient(publisher, icestorm.reference(), startReader = False)
    print("ok")
    publisherProc.startReader()

    subscriberProc.waitTestSuccess()
    publisherProc.waitTestSuccess()

    #
    # Destroy the topic.
    #
    sys.stdout.write("destroy topic... ")
    sys.stdout.flush()
    icestorm.admin("destroy single")
    print("ok")

    #
    # Shutdown icestorm.
    #
    icestorm.stop()

dotest("persistent")
dotest("transient")
dotest("replicated")

if TestUtil.appverifier:
    TestUtil.appVerifierAfterTestEnd([targets], cwd = os.getcwd())

sys.exit(0)
