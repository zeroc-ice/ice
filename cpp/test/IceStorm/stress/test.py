# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

#
# Publisher/subscriber test cases, publisher publishes on TestIceStorm1 instance(s) and
# the subscriber subscribes to the TestIceStorm2 instance(s)
#
pubSub=lambda si, pi, s={}, p={}: ClientServerTestCase(client=Publisher(args=s), server=Subscriber(args=p))

class IceStormStressTestCase(IceStormTestCase):

    def runClientSide(self, current):

        icestorm1 = [icestorm for icestorm in self.icestorm if icestorm.getInstanceName() == "TestIceStorm1"]
        icestorm2 = [icestorm for icestorm in self.icestorm if icestorm.getInstanceName() == "TestIceStorm2"]

        def doTest(subOpts, pubOpts):
            # Create the subscribers
            subscribers = []
            for (instanceName, opts) in subOpts if type(subOpts) == list else [subOpts]:
                subscribers.append(Subscriber(instanceName or "TestIceStorm1", args=opts.split(" ")))

            # Create the publisher
            publisher = Publisher("TestIceStorm1", args=pubOpts.split(" "))

            # Run the test case
            ClientServerTestCase(client=publisher, servers=subscribers).run(current)

        current.write("setting up the topics...")
        self.runadmin(current, "create TestIceStorm1/fed1 TestIceStorm2/fed1")
        current.writeln("ok")

        current.write("Sending 5000 ordered events... ")
        doTest(("TestIceStorm1", '--events 5000 --qos "reliability,ordered"'), '--events 5000')
        current.writeln("ok")

        self.runadmin(current, "link TestIceStorm1/fed1 TestIceStorm2/fed1")
        current.write("Sending 5000 ordered events across a link... ")
        doTest(("TestIceStorm2", '--events 5000 --qos "reliability,ordered"'), '--events 5000')
        current.writeln("ok")

        self.runadmin(current, "unlink TestIceStorm1/fed1 TestIceStorm2/fed1")
        current.write("Sending 20000 unordered events... ")
        doTest(("TestIceStorm1", '--events 20000'), '--events 20000 --oneway')
        current.writeln("ok")

        self.runadmin(current, "link TestIceStorm1/fed1 TestIceStorm2/fed1")
        current.write("Sending 20000 unordered events across a link... ")
        doTest(("TestIceStorm2", '--events 20000'), '--events 20000 --oneway')
        current.writeln("ok")

        self.runadmin(current, "unlink TestIceStorm1/fed1 TestIceStorm2/fed1")
        current.write("Sending 20000 unordered batch events... ")
        doTest(("TestIceStorm1", '--events 20000 --qos "reliability,batch"'), '--events 20000 --oneway')
        current.writeln("ok")

        self.runadmin(current, "link TestIceStorm1/fed1 TestIceStorm2/fed1")
        current.write("Sending 20000 unordered batch events across a link... ")
        doTest(("TestIceStorm2", '--events 20000 --qos "reliability,batch"'), '--events 20000 --oneway')
        current.writeln("ok")

        self.runadmin(current, "unlink TestIceStorm1/fed1 TestIceStorm2/fed1")
        current.write("Sending 20000 unordered events with slow subscriber... ")
        doTest([("TestIceStorm1", '--events 2 --slow'),
                ("TestIceStorm1", '--events 20000 ')],
                '--events 20000 --oneway')
        current.writeln("ok")

        self.runadmin(current, "link TestIceStorm1/fed1 TestIceStorm2/fed1")
        current.write("Sending 20000 unordered events with slow subscriber & link... ")
        doTest([("TestIceStorm1", '--events 2 --slow'),
                ("TestIceStorm1", '--events 20000'),
                ("TestIceStorm2", '--events 2 --slow'),
                ("TestIceStorm2", '--events 20000')],
                '--events 20000 --oneway')
        current.writeln("ok")

        current.write("shutting down icestorm services... ")
        self.stopIceStorm(current)
        current.writeln("ok")

        current.write("starting icestorm services... ")
        #
        # The erratic tests emit lots of connection warnings so they are
        # disabled here. The IceStorm servers are stopped and restarted so the
        # settings will take effect.
        #
        self.startIceStorm(current)
        current.writeln("ok")

        self.runadmin(current, "unlink TestIceStorm1/fed1 TestIceStorm2/fed1")
        current.write("Sending 20000 unordered events with erratic subscriber... ")
        doTest([ ("TestIceStorm1", '--erratic 5 --qos "reliability,ordered" --events 20000'),
                 ("TestIceStorm1", '--erratic 5 --events 20000'),
                 ("TestIceStorm1", '--events 20000')],
                 '--events 20000 --oneway')
        current.writeln("ok")

        self.runadmin(current, "link TestIceStorm1/fed1 TestIceStorm2/fed1")
        current.write("Sending 20000 unordered events with erratic subscriber across a link... ")
        doTest([ ("TestIceStorm1", '--events 20000'),
                 ("TestIceStorm1", '--erratic 5 --qos "reliability,ordered" --events 20000 '),
                 ("TestIceStorm1", '--erratic 5 --events 20000 '),
                 ("TestIceStorm2", '--events 20000'),
                 ("TestIceStorm2", '--erratic 5 --qos "reliability,ordered" --events 20000 '),
                 ("TestIceStorm2", '--erratic 5 --events 20000 ')],
                 '--events 20000 --oneway ')
        current.writeln("ok")

        #
        # Shutdown icestorm.
        #
        current.write("shutting down icestorm services... ")
        self.stopIceStorm(current)
        current.writeln("ok")

        current.write("Sending 5000 ordered events with max queue size drop events... ")
        opts = " --IceStorm.Send.QueueSizeMax=2000 --IceStorm.Send.QueueSizeMaxPolicy=DropEvents"
        for s in icestorm1:
            s.start(current, args=opts.split(" "))
        doTest(("TestIceStorm1", '--events 5000 --qos "reliability,ordered" --maxQueueDropEvents=2000'),
               '--events 5000 --maxQueueTest')
        for s in icestorm1:
            s.shutdown(current)
            s.stop(current, True)
        current.writeln("ok")

        current.write("Sending 5000 ordered events with max queue size remove subscriber... ")
        opts = " --IceStorm.Send.QueueSizeMax=2000 --IceStorm.Send.QueueSizeMaxPolicy=RemoveSubscriber"
        for s in icestorm1:
            s.start(current, args=opts.split(" "))
        doTest(("TestIceStorm1", '--events 5000 --qos "reliability,ordered" --maxQueueRemoveSub=2000'),
               '--events 5000 --maxQueueTest')
        for s in icestorm1:
            s.shutdown(current)
            s.stop(current, True)
        current.writeln("ok")

TestSuite(__file__, [

    IceStormStressTestCase("persistent", icestorm=[IceStorm("TestIceStorm1", quiet=True),
                                                   IceStorm("TestIceStorm2", quiet=True, portnum=20)]),

    IceStormStressTestCase("replicated with replicated publisher", icestorm=
                           [IceStorm("TestIceStorm1", i, 3, quiet=True) for i in range(0,3)] +
                           [IceStorm("TestIceStorm2", i, 3, portnum=20, quiet=True) for i in range(0,3)]),

], multihost=False)
