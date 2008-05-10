#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
TestUtil.processCmdLine()

name = os.path.join("IceStorm", "stress")
testdir = os.path.dirname(os.path.abspath(__file__))

iceBox = TestUtil.getIceBox(testdir)
iceBoxAdmin = os.path.join(TestUtil.getCppBinDir(), "iceboxadmin")
iceStormAdmin = os.path.join(TestUtil.getCppBinDir(), "icestormadmin")

import IceStormUtil

def doTest(server1, server2, subOpts, pubOpts):
    global testdir

    publisher = os.path.join(testdir, "publisher")
    subscriber = os.path.join(testdir, "subscriber")

    subscriberPipes = []
    if type(subOpts) != type([]):
        subOpts = [ subOpts ]
    for opts in subOpts:
        # We don't want the subscribers to time out.
        pipe = TestUtil.startServer(subscriber, r' --Ice.ServerIdleTime=0 ' + opts)
        TestUtil.getServerPid(pipe)
        TestUtil.getAdapterReady(pipe)
        subscriberPipes.append(pipe)

    publisherPipe = TestUtil.startClient(publisher, server1.reference() + r' ' + pubOpts)

    TestUtil.printOutputFromPipe(publisherPipe)

    publisherStatus = TestUtil.closePipe(publisherPipe)
    if publisherStatus:
        print "(publisher failed)",
        return publisherStatus
    for p in subscriberPipes:
        try:
            sys.stdout.flush()
            subscriberStatus = TestUtil.specificServerStatus(p)
        except:
            print "(subscriber failed)",
            return 1
        if subscriberStatus:
            print "(subscriber failed)",
            return subscriberStatus

    return 0

def runAdmin(cmd, desc = None):
    global iceStormAdmin
    global iceStormAdminReference
    if desc:
        print desc,
        sys.stdout.flush()
    pipe = TestUtil.startClient(iceStormAdmin, adminIceStormReference + r' -e "' + cmd + '"')
    status = TestUtil.closePipe(pipe)
    if status:
        TestUtil.killServers()
        sys.exit(1)
    if desc:
        print "ok"

def runtest(type):
    # Clear the idle timeout otherwise the IceBox ThreadPool will timeout.
    server1 = IceStormUtil.init(toplevel, testdir, type, dbDir = "db", instanceName = "TestIceStorm1", port = 12000)
    server2 = IceStormUtil.init(toplevel, testdir, type, dbDir = "db2", instanceName = "TestIceStorm2", port = 12500)
    global adminIceStormReference
    adminIceStormReference = ' --IceStormAdmin.TopicManager.Proxy="%s" --IceStormAdmin.TopicManager.Proxy2="%s"' % (
        server1.proxy(), server2.proxy())

    print "starting icestorm services...",
    sys.stdout.flush()
    server1.start(echo=False)
    server2.start(echo=False)
    print "ok"

    runAdmin("create TestIceStorm1/fed1 TestIceStorm2/fed1", "setting up the topics...")

    print "Sending 5000 ordered events... ",
    sys.stdout.flush()
    status = doTest(server1, server2, '--events 5000 --qos "reliability,ordered" ' + server1.reference(), '--events 5000')
    if status:
	print "failed!"
	TestUtil.killServers()
	sys.exit(1)
    print "ok"

    runAdmin("link TestIceStorm1/fed1 TestIceStorm2/fed1")
    print "Sending 5000 ordered events across a link... ",
    sys.stdout.flush()
    status = doTest(server1, server2, '--events 5000 --qos "reliability,ordered" ' + server2.reference(), '--events 5000')
    if status:
	TestUtil.killServers()
	sys.exit(1)
    print "ok"

    runAdmin("unlink TestIceStorm1/fed1 TestIceStorm2/fed1")
    print "Sending 20000 unordered events... ",
    sys.stdout.flush()
    status = doTest(server1, server2, '--events 20000 ' + server1.reference(), '--events 20000 --oneway')
    if status:
	print "failed!"
	TestUtil.killServers()
	sys.exit(1)
    print "ok"

    runAdmin("link TestIceStorm1/fed1 TestIceStorm2/fed1")
    print "Sending 20000 unordered events across a link... ",
    sys.stdout.flush()
    status = doTest(server1, server2, '--events 20000 ' + server2.reference(), '--events 20000 --oneway')
    if status:
	TestUtil.killServers()
	sys.exit(1)
    print "ok"

    runAdmin("unlink TestIceStorm1/fed1 TestIceStorm2/fed1")
    print "Sending 20000 unordered batch events... ",
    sys.stdout.flush()
    status = doTest(server1, server2, '--events 20000 --qos "reliability,batch" ' + server1.reference(), '--events 20000 --oneway')
    if status:
	print "failed!"
	TestUtil.killServers()
	sys.exit(1)
    print "ok"

    runAdmin("link TestIceStorm1/fed1 TestIceStorm2/fed1")
    print "Sending 20000 unordered batch events across a link... ",
    sys.stdout.flush()
    status = doTest(server1, server2, '--events 20000 --qos "reliability,batch" ' + server2.reference(), '--events 20000 --oneway')
    if status:
	TestUtil.killServers()
	sys.exit(1)
    print "ok"

    runAdmin("unlink TestIceStorm1/fed1 TestIceStorm2/fed1")
    print "Sending 20000 unordered events with slow subscriber... ",
    status = doTest(server1, server2, ['--events 2 --slow ' + server1.reference(), '--events 20000 ' + server1.reference()], '--events 20000 --oneway')
    if status:
	print "failed!"
	TestUtil.killServers()
	sys.exit(1)
    print "ok"

    runAdmin("link TestIceStorm1/fed1 TestIceStorm2/fed1")
    print "Sending 20000 unordered events with slow subscriber & link... ",
    status = doTest(server1, server2, ['--events 2 --slow' + server1.reference(), '--events 20000' + server1.reference(), '--events 2 --slow' + server2.reference(), '--events 20000' + server2.reference()], '--events 20000 --oneway')
    if status:
	print "failed!"
	TestUtil.killServers()
	sys.exit(1)
    print "ok"


    print "shutting down icestorm services...",
    sys.stdout.flush()
    server1.stop()
    server2.stop()
    print "ok"

    print "starting icestorm services...",
    sys.stdout.flush()
    #
    # The erratic tests emit lots of connection warnings so they are
    # disabled here. The IceStorm servers are stopped and restarted so the
    # settings will take effect.
    #
    server1.start(echo=False, additionalOptions = ' --Ice.Warn.Connections=0')
    server2.start(echo=False, additionalOptions = ' --Ice.Warn.Connections=0')
    print "ok"

    runAdmin("unlink TestIceStorm1/fed1 TestIceStorm2/fed1")

    print "Sending 20000 unordered events with erratic subscriber... ",
    sys.stdout.flush()
    status = doTest(server1, server2, \
        [ '--erratic 5 --qos "reliability,ordered" --events 20000' + server1.reference(), \
          '--erratic 5 --events 20000' + server1.reference(), \
          '--events 20000' + server1.reference()], \
          '--events 20000 --oneway')
    if status:
        print "failed!"
        TestUtil.killServers()
        sys.exit(1)
    print "ok"

    runAdmin("link TestIceStorm1/fed1 TestIceStorm2/fed1")
    print "Sending 20000 unordered events with erratic subscriber across a link... ",
    sys.stdout.flush()
    status = doTest(server1, server2,  \
         [ '--events 20000' + server1.reference(), \
           '--erratic 5 --qos "reliability,ordered" --events 20000 ' + server1.reference(), \
           '--erratic 5 --events 20000 ' + server1.reference(), \
           '--events 20000' + server2.reference(), \
           '--erratic 5 --qos "reliability,ordered" --events 20000 ' + server2.reference(), \
           '--erratic 5 --events 20000 ' + server2.reference()], \
           '--events 20000 --oneway ')
    if status:
        print "failed!"
        TestUtil.killServers()
        sys.exit(1)
    print "ok"

    #
    # Shutdown icestorm.
    #
    print "shutting down icestorm services...",
    sys.stdout.flush()
    server1.stop()
    server2.stop()
    print "ok"

    if TestUtil.serverStatus():
        TestUtil.killServers()
        sys.exit(1)

runtest("persistent")
runtest("replicated")

sys.exit(0)
