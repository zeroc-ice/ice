# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

#
# Publisher/subscriber test cases, publisher publishes on TestIceStorm1 instance(s) and
# the subscriber subscribes to the TestIceStorm2 instance(s)
#
pub1Sub2Oneway=ClientServerTestCase(client=Publisher("TestIceStorm1"), server=Subscriber("TestIceStorm2"))
pub1Sub2Batch=ClientServerTestCase(client=Publisher("TestIceStorm1"), server=Subscriber("TestIceStorm2", args=["-b"]))

pub1Sub1Oneway=ClientServerTestCase(client=Publisher("TestIceStorm1"), server=Subscriber("TestIceStorm1"))

class IceStormFederation2TestCase(IceStormTestCase):

    def runClientSide(self, current):

        current.write("setting up the topics... ")
        self.runadmin(current, "create TestIceStorm1/fed1 TestIceStorm2/fed1; link TestIceStorm1/fed1 TestIceStorm2/fed1")
        current.writeln("ok")

        #
        # Test oneway subscribers.
        #
        current.write("testing federation with oneway subscribers... ")
        pub1Sub2Oneway.run(current)
        current.writeln("ok")

        #
        # Test batch oneway subscribers.
        #
        current.write("testing federation with batch subscribers... ")
        pub1Sub2Batch.run(current)
        current.writeln("ok")

        #
        # Test #2:
        #
        # Stop and restart the service and repeat the test. This ensures that
        # the database is correct.
        #
        current.write("restarting services to ensure that the database content is preserved... ")
        self.restartIceStorm(current)
        current.writeln("ok")

        #
        # Test oneway subscribers.
        #
        current.write("retesting federation with oneway subscribers... ")
        pub1Sub2Oneway.run(current)
        current.writeln("ok")

        #
        # Test batch oneway subscribers.
        #
        current.write("retesting federation with batch subscribers... ")
        pub1Sub2Batch.run(current)
        current.writeln("ok")

        #
        # Shutdown icestorm.
        #
        self.stopIceStorm(current)

        icestorm1 = [icestorm for icestorm in self.icestorm if icestorm.getInstanceName() == "TestIceStorm1"]
        icestorm2 = [icestorm for icestorm in self.icestorm if icestorm.getInstanceName() == "TestIceStorm2"]

        #
        # Restart the first server and publish some events. Attach a
        # subscriber to the channel and make sure the events are received.
        #
        # Then re-start the linked downstream server and publish the events.
        # Ensure they are received by the linked server.
        #
        if self.getName().find("replicated") == -1:

            current.write("restarting only one IceStorm server... ")
            icestorm1[0].start(current)
            current.writeln("ok")

            #
            # Test oneway subscribers.
            #
            current.write("testing that the federation link reports an error... ")
            pub1Sub1Oneway.run(current)

            # Give some time for the output to be sent.
            time.sleep(2)

            icestorm1[0].expect(current, "topic.fed1.*subscriber offline")
            current.writeln("ok")

            current.write("starting downstream icestorm server... ")
            icestorm2[0].start(current)
            current.writeln("ok")

            #
            # Need to sleep for at least the discard interval.
            #
            time.sleep(3)

            #
            # Test oneway subscribers.
            #
            current.write("testing link is reestablished... ")
            pub1Sub2Oneway.run(current)
            current.writeln("ok")

            try:
                icestorm1[0].expect(current, "topic.fed1.*subscriber offline", timeout=1)
                assert False
            except Expect.TIMEOUT:
                pass

            self.stopIceStorm(current)

        #
        # Test #4:
        #
        # Trash the TestIceStorm2 database. Then restart the servers and
        # verify that the link is removed.
        #
        current.write("destroying the downstream IceStorm service database... ")
        for s in icestorm2:
            s.teardown(current, True)
            s.setup(current)

        current.writeln("ok")

        current.write("restarting IceStorm servers... ")
        self.startIceStorm(current)
        current.writeln("ok")

        current.write("checking link still exists... ")
        line = self.runadmin(current, "links TestIceStorm1", quiet=True)
        if not re.compile("fed1 with cost 0").search(line):
            raise RuntimeError("unexpected output (`{0}')".format(line))
        current.writeln("ok")

        current.write("publishing some events... ")
        # The publisher must be run twice because all the events can be
        # sent out in one batch to the linked subscriber which means that
        # the link is not reaped until the next batch is
        # sent. Furthermore, with a replicated IceStorm both sets of
        # events must be set to the same replica.
        Publisher("TestIceStorm1", args=["--count", 2]).run(current)
        current.writeln("ok")

        # Give the unsubscription time to propagate.
        time.sleep(1)

        # Verify that the link has disappeared.
        current.write("verifying that the link has been destroyed... ")
        line = self.runadmin(current, "links TestIceStorm1")
        nRetry = 5
        while len(line) > 0 and nRetry > 0:
            line = self.runadmin(current, "links TestIceStorm1")
            time.sleep(1) # Give more time for unsubscription to propagate.
            nRetry -= 1
        if len(line) > 0:
            raise RuntimeError("unexpected output (`{0}')".format(line))
        current.writeln("ok")

        #
        # Destroy the remaining topic.
        #
        current.write("destroying topics... ")
        self.runadmin(current, "destroy TestIceStorm1/fed1links TestIceStorm1", quiet=True)
        current.writeln("ok")

        self.stopIceStorm(current)

# Override ReplicatedPublishEndpoints property to empty for testing without replicated publisher
props = { 'IceStorm.Discard.Interval' : 2 }
nonRepProps = { 'IceStorm.Discard.Interval' : 2, 'IceStorm.ReplicatedPublishEndpoints' : '' }

TestSuite(__file__, [

    IceStormFederation2TestCase("persistent", icestorm=
                                [IceStorm("TestIceStorm1", quiet=True, props=props),
                                 IceStorm("TestIceStorm2", quiet=True,portnum=20, props=props)]),

    IceStormFederation2TestCase("replicated with non-replicated publisher", icestorm=
                                [IceStorm("TestIceStorm1", i, 3, quiet=True, props=nonRepProps) for i in range(0,3)] +
                                [IceStorm("TestIceStorm2", i, 3, quiet=True, portnum=20, props=nonRepProps) for i in range(0,3)]),

    IceStormFederation2TestCase("replicated with replicated publisher", icestorm=
                                [IceStorm("TestIceStorm1", i, 3, quiet=True, props=props) for i in range(0,3)] +
                                [IceStorm("TestIceStorm2", i, 3, quiet=True, portnum=20, props=props) for i in range(0,3)]),
], multihost=False)
