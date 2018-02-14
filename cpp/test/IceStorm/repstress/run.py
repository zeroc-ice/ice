#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, time, re

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
control = os.path.join(os.getcwd(), "control")

targets = []
if TestUtil.appverifier:
    targets = [TestUtil.getIceBox(), publisher, subscriber, control, TestUtil.getIceBoxAdmin(), \
               TestUtil.getIceStormAdmin()]
    TestUtil.setAppVerifierSettings(targets, cwd = os.getcwd())

def runcontrol(proxy):
    proc = TestUtil.startClient(control, ' "%s"' % proxy)
    proc.waitTestSuccess()

icestorm = IceStormUtil.init(TestUtil.toplevel, os.getcwd(), "replicated", replicatedPublisher=True, additional =
                             ' --IceStorm.Election.MasterTimeout=2' +
                             ' --IceStorm.Election.ElectionTimeout=2' +
                             ' --IceStorm.Election.ResponseTimeout=2')
icestorm.start()

sys.stdout.write("creating topic... ")
sys.stdout.flush()
icestorm.admin("create single")
print("ok")

sys.stdout.write("running subscriber... ")
sys.stdout.flush()
subscriberProc = TestUtil.startServer(subscriber, ' --Ice.ServerIdleTime=0 ' + icestorm.reference(), echo = False)
subscriberProc.expect("([^\n]+)\n")
subControl = subscriberProc.match.group(1)
print("ok")

sys.stdout.write("running publisher... ")
sys.stdout.flush()
publisherProc = TestUtil.startServer(publisher, ' --Ice.ServerIdleTime=0 ' + icestorm.reference(), echo = False)
publisherProc.expect("([^\n]+)\n")
pubControl = publisherProc.match.group(1)
print("ok")

time.sleep(2)

for i in range(0, 3):
    # 0, 1
    sys.stdout.write("stopping replica 2 (0, 1 running)... ")
    sys.stdout.flush()
    icestorm.stopReplica(2)
    print("ok")
    time.sleep(2)

    # 1, 2
    sys.stdout.write("starting 2, stopping 0 (1, 2 running)... ")
    sys.stdout.flush()
    icestorm.startReplica(2, echo=False)
    icestorm.stopReplica(0)
    print("ok")
    # This waits for the replication to startup
    #icestorm.admin("list")
    time.sleep(2)

    # 0, 2
    sys.stdout.write("starting 0, stopping 1 (0, 2 running)... ")
    sys.stdout.flush()
    icestorm.startReplica(0, echo=False)
    icestorm.stopReplica(1)
    print("ok")
    # This waits for the replication to startup
    #icestorm.admin("list")
    time.sleep(2)

    sys.stdout.write("starting 1 (all running)... ")
    sys.stdout.flush()
    icestorm.startReplica(1, echo=False)
    print("ok")
    # This waits for the replication to startup
    #icestorm.admin("list")

    time.sleep(2)

sys.stdout.write("stopping publisher... ")
sys.stdout.flush()
runcontrol(pubControl)
publisherProc.expect("([^\n]+)\n")
publisherCount = publisherProc.match.group(1)
publisherProc.waitTestSuccess()
print("ok")

sys.stdout.write("stopping replicas... ")
sys.stdout.flush()
icestorm.stop()
print("ok")

sys.stdout.write("stopping subscriber... ")
sys.stdout.flush()
runcontrol(subControl)
subscriberProc.expect("([^\n]+)\n")
subscriberCount = subscriberProc.match.group(1)
subscriberProc.waitTestSuccess()
print("ok")

print("publisher published %s events, subscriber received %s events" % (publisherCount, subscriberCount))

if TestUtil.appverifier:
    TestUtil.appVerifierAfterTestEnd(targets, cwd = os.getcwd())
