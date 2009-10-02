#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
    raise "can't find toplevel directory!"
sys.path.append(os.path.join(path[0]))
from scripts import *

iceStormAdmin = ""
if TestUtil.isNoServices():
    iceStormAdmin = os.path.join(TestUtil.getServiceDir(), "icestormadmin")
else:
    iceStormAdmin = os.path.join(TestUtil.getCppBinDir(), "icestormadmin")

publisher = os.path.join(os.getcwd(), "publisher")
subscriber = os.path.join(os.getcwd(), "subscriber")

def admin(ref, command):
    proc = TestUtil.startClient(iceStormAdmin, ref + ' -e "%s"' % command, echo = False)
    proc.waitTestSuccess()
    return proc.buf

def runPublisher(icestorm1, opt = ""):
    proc = TestUtil.startClient(publisher, opt + icestorm1.reference())
    proc.waitTestSuccess()

def doTest(icestorm1, icestorm2, batch, subscriberRef = None):
    if batch:
        name = "batch subscriber"
        batchOptions = " -b"
    else:
        name = "subscriber"
        batchOptions = ""

    if subscriberRef == None:
        subscriberRef = icestorm2.reference()

    subscriberProc = TestUtil.startServer(subscriber, batchOptions + subscriberRef)

    #
    # Start the publisher. This should publish events which eventually
    # causes subscriber to terminate.
    #
    runPublisher(icestorm1)

    subscriberProc.waitTestSuccess(timeout=30)

#
# Test #1:
#
# create a cross service link fed1->fed2 and ensure the events are
# published between them correctly.
#
def runtest(type, **args):
    icestorm1 = IceStormUtil.init(TestUtil.toplevel, os.getcwd(), type, additional = '--IceStorm.Discard.Interval=2',
                                  dbDir = "db", instanceName = "TestIceStorm1", port = 12000, **args)
    icestorm1.start()
    icestorm2 = IceStormUtil.init(TestUtil.toplevel, os.getcwd(), type, additional = '--IceStorm.Discard.Interval=2',
                                  dbDir = "db2", instanceName = "TestIceStorm2", port = 12500, **args)
    icestorm2.start()

    adminIceStormReference = ' --IceStormAdmin.TopicManager.Proxy="%s" --IceStormAdmin.TopicManager.Proxy2="%s"' % (
        icestorm1.proxy(), icestorm2.proxy())

    print "setting up the topics...",
    sys.stdout.flush()
    admin(adminIceStormReference, "create TestIceStorm1/fed1 TestIceStorm2/fed1; link TestIceStorm1/fed1 TestIceStorm2/fed1")
    print "ok"

    #
    # Test oneway subscribers.
    #
    print "testing federation with oneway subscribers...",
    sys.stdout.flush()
    doTest(icestorm1, icestorm2, 0)
    print "ok"

    #
    # Test batch oneway subscribers.
    #
    print "testing federation with batch subscribers...",
    sys.stdout.flush()
    doTest(icestorm1, icestorm2, 1)
    print "ok"

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
    icestorm1.stop()
    icestorm2.stop()

    icestorm1.start(echo=False)
    icestorm2.start(echo=False)
    print "ok"

    #
    # Test oneway subscribers.
    #
    print "retesting federation with oneway subscribers... ",
    sys.stdout.flush()
    doTest(icestorm1, icestorm2, 0)
    print "ok"

    #
    # Test batch oneway subscribers.
    #
    print "retesting federation with batch subscribers... ",
    sys.stdout.flush()
    doTest(icestorm1, icestorm2, 1)
    print "ok"

    #
    # Shutdown icestorm.
    #
    icestorm1.stop()
    icestorm2.stop()

    #
    # Restart the first server and publish some events. Attach a
    # subscriber to the channel and make sure the events are received.
    #
    # Then re-start the linked downstream server and publish the events.
    # Ensure they are received by the linked server.
    #
    if type != "replicated":
        print "restarting only one IceStorm server...",
        sys.stdout.flush()
        proc = icestorm1.start(echo=False)

        #proc.expect("topic.fed1.*subscriber offline")
        #proc.expect("connection refused")
        print "ok"

        #
        # Test oneway subscribers.
        #
        print "testing that the federation link reports an error...",
        sys.stdout.flush()
        doTest(icestorm1, icestorm2, 0, icestorm1.reference())

	# Give some time for the output to be sent.
	time.sleep(2)

        proc.expect("topic.fed1.*subscriber offline")
        print "ok"

        print "starting downstream icestorm server...",
        sys.stdout.flush()
        icestorm2.start(echo=False)
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
        doTest(icestorm1, icestorm2, 0)
        print "ok"

        try:
            proc.expect("topic.fed1.*subscriber offline")
            assert False
        except Expect.TIMEOUT:
            pass

        icestorm1.stop()
        icestorm2.stop()

    #
    # Test #4:
    #
    # Trash the TestIceStorm2 database. Then restart the servers and
    # verify that the link is removed.
    #
    print "destroying the downstream IceStorm service database...",
    sys.stdout.flush()
    icestorm2.clean()

    print "ok"

    print "restarting IceStorm servers...",
    sys.stdout.flush()
    icestorm1.start(echo = False)
    icestorm2.start(echo = False)
    print "ok"

    print "checking link still exists...",
    line = admin(adminIceStormReference, "links TestIceStorm1")
    if not re.compile("fed1 with cost 0").search(line):
        print line
        sys.exit(1)
    print "ok"

    print "publishing some events...",
    sys.stdout.flush()
    # The publisher must be run twice because all the events can be
    # sent out in one batch to the linked subscriber which means that
    # the link is not reaped until the next batch is
    # sent. Furthermore, with a replicated IceStorm both sets of
    # events must be set to the same replica.
    runPublisher(icestorm1, opt = " --count 2")
    print "ok"

    # Give the unsubscription time to propagate.
    time.sleep(1)

    # Verify that the link has disappeared.
    print "verifying that the link has been destroyed...",
    sys.stdout.flush()
    line = admin(adminIceStormReference, "links TestIceStorm1")
    nRetry = 5
    while len(line) > 0 and nRetry > 0:
        line = admin(adminIceStormReference, "links TestIceStorm1")
        time.sleep(1) # Give more time for unsubscription to propagate.
        nRetry -= 1
    if len(line) > 0:
        print line
        TestUtil.killServers()
        sys.exit(1)
    print "ok"

    #
    # Destroy the remaining topic.
    #
    print "destroying topics...",
    admin(adminIceStormReference, "destroy TestIceStorm1/fed1")
    print "ok"

    #
    # Shutdown icestorm.
    #
    print "shutting down icestorm services...",
    sys.stdout.flush()
    icestorm1.stop()
    icestorm2.stop()
    print "ok"

runtest("persistent")
runtest("replicated", replicatedPublisher = False)
runtest("replicated", replicatedPublisher = True)

TestUtil.cleanup()
sys.exit(0)
