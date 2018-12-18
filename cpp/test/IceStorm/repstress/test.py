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
# Make sure IceStorm and the subscriber use the same buffer size for
# sending/receiving datagrams. This ensures the test works with bogus
# OS configurations where the reicever buffer size is smaller than the
# send buffer size (causing the received messages to be
# truncated). See also bug #6070.
#
props = {
    "IceStorm.Election.MasterTimeout" : 2,
    "IceStorm.Election.ElectionTimeout" : 2,
    "IceStorm.Election.ResponseTimeout" : 2,
    "Ice.Warn.Dispatch" : 0
}

icestorm = [ IceStorm(replica=i, nreplicas=3, props = props) for i in range(0,3) ]

class IceStormRepStressTestCase(IceStormTestCase):

    def runClientSide(self, current):

        def stopReplica(num):
            self.icestorm[num].shutdown(current)
            self.icestorm[num].stop(current, True)

        def startReplica(num):
            self.icestorm[num].start(current)

        current.write("creating topic... ")
        self.runadmin(current, "create single")
        current.writeln("ok")

        current.write("running subscriber... ")
        subscriber = Subscriber(quiet=True)
        subscriber.start(current)
        subscriber.expect(current, "([^\n]+)\n")
        subControl = subscriber.getMatch(current).group(1)
        current.writeln("ok")

        current.write("running publisher... ")
        publisher = Publisher(quiet=True)
        publisher.start(current)
        publisher.expect(current, "([^\n]+)\n")
        pubControl = publisher.getMatch(current).group(1)
        current.writeln("ok")

        time.sleep(2)

        for i in range(0, 3):
            # 0, 1
            current.write("stopping replica 2 (0, 1 running)... ")
            stopReplica(2)
            current.writeln("ok")
            time.sleep(2)

            # 1, 2
            current.write("starting 2, stopping 0 (1, 2 running)... ")
            startReplica(2)
            stopReplica(0)
            current.writeln("ok")
            # This waits for the replication to startup
            #self.runadmin(current, "list")
            time.sleep(2)

            # 0, 2
            current.write("starting 0, stopping 1 (0, 2 running)... ")
            startReplica(0)
            stopReplica(1)
            current.writeln("ok")
            # This waits for the replication to startup
            #self.runadmin(current, "list")
            time.sleep(2)

            current.write("starting 1 (all running)... ")
            startReplica(1)
            current.writeln("ok")
            # This waits for the replication to startup
            #self.runadmin(current, "list")
            time.sleep(2)

        current.write("stopping publisher... ")
        Client(exe="control", args=[pubControl]).run(current)
        publisher.expect(current, "([^\n]+)\n")
        publisherCount = publisher.getMatch(current).group(1)
        publisher.stop(current, True)
        current.writeln("ok")

        current.write("stopping replicas... ")
        self.stopIceStorm(current)
        current.writeln("ok")

        current.write("stopping subscriber... ")
        Client(exe="control", args=[subControl]).run(current)
        subscriber.expect(current, "([^\n]+)\n")
        subscriberCount = subscriber.getMatch(current).group(1)
        subscriber.stop(current, True)
        current.writeln("ok")

        current.writeln("publisher published %s events, subscriber received %s events" % (publisherCount, subscriberCount))

TestSuite(__file__,
          [ IceStormRepStressTestCase("replicated", icestorm=icestorm) ],
          options={ "ipv6" : [False] },
          multihost=False, runOnMainThread=True)
