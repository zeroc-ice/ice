#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, time, threading, re

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil, IceStormUtil

iceStormAdmin = TestUtil.getIceStormAdmin()

publisher = os.path.join(os.getcwd(), "publisher")
subscriber = os.path.join(os.getcwd(), "subscriber")

targets = []
if TestUtil.appverifier:
    targets = [TestUtil.getIceBox(), publisher, subscriber, TestUtil.getIceBoxAdmin(), \
               TestUtil.getIceStormAdmin()]
    TestUtil.setAppVerifierSettings(targets, cwd = os.getcwd())

def doTest(server1, subOpts, pubOpts):

    subscriberProcs = []
    if type(subOpts) != type([]):
        subOpts = [ subOpts ]
    for opts in subOpts:
        # We don't want the subscribers to time out.
        proc = TestUtil.startServer(subscriber, ' --Ice.ServerIdleTime=0 ' + opts)
        subscriberProcs.append(proc)

    publisherProc = TestUtil.startClient(publisher, server1.reference() + r' ' + pubOpts)
    publisherProc.waitTestSuccess()
    for p in subscriberProcs:
        sys.stdout.flush()
        p.waitTestSuccess()

    return 0

def runAdmin(cmd, desc = None):
    global iceStormAdmin
    global iceStormAdminReference
    if desc:
        sys.stdout.write(desc + " ")
        sys.stdout.flush()
    proc = TestUtil.startClient(iceStormAdmin, adminIceStormReference + r' -e "%s"' % cmd, startReader = True)
    proc.waitTestSuccess()
    if desc:
        print("ok")

def runtest(type):
    # Clear the idle timeout otherwise the IceBox ThreadPool will timeout.
    server1 = IceStormUtil.init(TestUtil.toplevel, os.getcwd(), type, dbDir = "db", instanceName = "TestIceStorm1",
                                port = 12000)
    server2 = IceStormUtil.init(TestUtil.toplevel, os.getcwd(), type, dbDir = "db2", instanceName = "TestIceStorm2",
                                port = 12500)
    global adminIceStormReference
    adminIceStormReference = ' --IceStormAdmin.TopicManager.Proxy="%s" --IceStormAdmin.TopicManager.Proxy2="%s"' % (
        server1.proxy(), server2.proxy())

    sys.stdout.write("starting icestorm services... ")
    sys.stdout.flush()
    server1.start(echo=False)
    server2.start(echo=False)
    print("ok")

    runAdmin("create TestIceStorm1/fed1 TestIceStorm2/fed1", "setting up the topics...")

    sys.stdout.write("Sending 5000 ordered events... ")
    sys.stdout.flush()
    doTest(server1, '--events 5000 --qos "reliability,ordered" ' + server1.reference(), '--events 5000')
    print("ok")

    runAdmin("link TestIceStorm1/fed1 TestIceStorm2/fed1")
    sys.stdout.write("Sending 5000 ordered events across a link... ")
    sys.stdout.flush()
    doTest(server1, '--events 5000 --qos "reliability,ordered" ' + server2.reference(), '--events 5000')
    print("ok")

    runAdmin("unlink TestIceStorm1/fed1 TestIceStorm2/fed1")
    sys.stdout.write("Sending 20000 unordered events... ")
    sys.stdout.flush()
    doTest(server1, '--events 20000 ' + server1.reference(), '--events 20000 --oneway')
    print("ok")

    runAdmin("link TestIceStorm1/fed1 TestIceStorm2/fed1")
    sys.stdout.write("Sending 20000 unordered events across a link... ")
    sys.stdout.flush()
    doTest(server1, '--events 20000 ' + server2.reference(), '--events 20000 --oneway')
    print("ok")

    runAdmin("unlink TestIceStorm1/fed1 TestIceStorm2/fed1")
    sys.stdout.write("Sending 20000 unordered batch events... ")
    sys.stdout.flush()
    doTest(server1, '--events 20000 --qos "reliability,batch" ' + server1.reference(), '--events 20000 --oneway')
    print("ok")

    runAdmin("link TestIceStorm1/fed1 TestIceStorm2/fed1")
    sys.stdout.write("Sending 20000 unordered batch events across a link... ")
    sys.stdout.flush()
    doTest(server1, '--events 20000 --qos "reliability,batch" ' + server2.reference(), '--events 20000 --oneway')
    print("ok")

    runAdmin("unlink TestIceStorm1/fed1 TestIceStorm2/fed1")
    sys.stdout.write("Sending 20000 unordered events with slow subscriber... ")
    sys.stdout.flush()
    doTest(server1, ['--events 2 --slow ' + server1.reference(), '--events 20000 ' + server1.reference()], '--events 20000 --oneway')
    print("ok")

    runAdmin("link TestIceStorm1/fed1 TestIceStorm2/fed1")
    sys.stdout.write("Sending 20000 unordered events with slow subscriber & link... ")
    sys.stdout.flush()
    doTest(server1, ['--events 2 --slow' + server1.reference(), '--events 20000' + server1.reference(), '--events 2 --slow' + server2.reference(), '--events 20000' + server2.reference()], '--events 20000 --oneway')
    print("ok")

    sys.stdout.write("shutting down icestorm services... ")
    sys.stdout.flush()
    server1.stop()
    server2.stop()
    print("ok")

    sys.stdout.write("starting icestorm services... ")
    sys.stdout.flush()
    #
    # The erratic tests emit lots of connection warnings so they are
    # disabled here. The IceStorm servers are stopped and restarted so the
    # settings will take effect.
    #
    server1.start(echo=False, additionalOptions = ' --Ice.Warn.Connections=0')
    server2.start(echo=False, additionalOptions = ' --Ice.Warn.Connections=0')
    print("ok")

    runAdmin("unlink TestIceStorm1/fed1 TestIceStorm2/fed1")

    sys.stdout.write("Sending 20000 unordered events with erratic subscriber... ")
    sys.stdout.flush()
    doTest(server1,
        [ '--erratic 5 --qos "reliability,ordered" --events 20000' + server1.reference(),
          '--erratic 5 --events 20000' + server1.reference(),
          '--events 20000' + server1.reference()],
          '--events 20000 --oneway')
    print("ok")

    runAdmin("link TestIceStorm1/fed1 TestIceStorm2/fed1")
    sys.stdout.write("Sending 20000 unordered events with erratic subscriber across a link... ")
    sys.stdout.flush()
    doTest(server1,
         [ '--events 20000' + server1.reference(),
           '--erratic 5 --qos "reliability,ordered" --events 20000 ' + server1.reference(),
           '--erratic 5 --events 20000 ' + server1.reference(),
           '--events 20000' + server2.reference(),
           '--erratic 5 --qos "reliability,ordered" --events 20000 ' + server2.reference(),
           '--erratic 5 --events 20000 ' + server2.reference()],
           '--events 20000 --oneway ')
    print("ok")

    #
    # Shutdown icestorm.
    #
    sys.stdout.write("shutting down icestorm services... ")
    sys.stdout.flush()
    server1.stop()
    server2.stop()
    print("ok")

    sys.stdout.write("Sending 5000 ordered events with max queue size drop events... ")
    sys.stdout.flush()
    server1.start(echo=False,
                  additionalOptions = ' --IceStorm.Send.QueueSizeMax=400 --IceStorm.Send.QueueSizeMaxPolicy=DropEvents')
    doTest(server1,
           '--events 5000 --qos "reliability,ordered" --maxQueueDropEvents=400 ' + server1.reference(),
           '--events 5000')
    server1.stop()
    print("ok")

    sys.stdout.write("Sending 5000 ordered events with max queue size remove subscriber... ")
    sys.stdout.flush()
    server1.start(echo=False,
                  additionalOptions = ' --IceStorm.Send.QueueSizeMax=400 --IceStorm.Send.QueueSizeMaxPolicy=RemoveSubscriber')
    doTest(server1,
           '--events 500 --qos "reliability,ordered" --maxQueueRemoveSub=400 ' + server1.reference(),
           '--events 500')
    server1.stop()
    print("ok")



runtest("persistent")
runtest("replicated")

if TestUtil.appverifier:
    TestUtil.appVerifierAfterTestEnd(targets, cwd = os.getcwd())
