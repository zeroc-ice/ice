# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
    "IceStorm.Election.ResponseTimeout" : 2
}

icestorm = [ IceStorm(replica=i, nreplicas=3, props = props) for i in range(0,3) ]

class IceStormRep1TestCase(IceStormTestCase):

    def runClientSide(self, current):

        def checkExpect(output, expect):
            if not expect:
                return

            if type(expect) == str:
                expect = [expect]

            for e in expect:
                if output.find(e) >= 0:
                    return
            else:
                raise RuntimeError("unexected output `{0}' (expected `{1}')".format(output, expect))

        def adminForReplica(replica, cmd, expect):
            checkExpect(self.runadmin(current, cmd, instance=self.icestorm[replica], quiet=True, exitstatus=1),
                        expect)

        def stopReplica(num):
            self.icestorm[num].shutdown(current)
            self.icestorm[num].stop(current, True)

        def startReplica(num):
            self.icestorm[num].start(current)

        def runtest(s="", p=""):
            ClientServerTestCase(client=Publisher(args=p.split(" ")),
                                 server=Subscriber(args=s.split(" "))).run(current)

        def runsub2(replica=None, expect=None):
            subscriber = Subscriber(exe="sub",
                                    instance=None if replica is None else self.icestorm[replica],
                                    args=["--id", "foo"],
                                    readyCount=0,
                                    quiet=True)
            subscriber.run(current, exitstatus=1 if expect else 0)
            checkExpect(subscriber.getOutput(current), expect)

        def rununsub2(replica=None, expect=None):
            sub = Subscriber(exe="sub",
                             instance=None if replica is None else self.icestorm[replica],
                             args=["--id", "foo"],
                             readyCount=0,
                             quiet=True)

            if replica is None:
                sub.run(current, args=["--unsub"])
            # Else we first subscribe to this replica, then unsub. We
            # shouldn't get an AlreadySubscribedException.
            sub.run(current, exitstatus=1 if expect else 0)
            if expect:
                checkExpect(sub.getOutput(current), expect)
                return

            sub.run(current, args=["--unsub"])

        current.write("testing topic creation across replicas... ")
        self.runadmin(current, "create single")
        for replica in range(0, 3):
            adminForReplica(replica, "create single", "error: topic `single' exists")
        current.writeln("ok")

        current.write("testing topic destruction across replicas... ")
        sys.stdout.flush()
        self.runadmin(current, "destroy single")

        for replica in range(0, 3):
            adminForReplica(replica, "destroy single", "error: couldn't find topic `single'")
        current.writeln("ok")

        current.write("testing topic creation without replica... ")

        stopReplica(0)

        self.runadmin(current, "create single")

        for replica in range(1, 3):
            adminForReplica(replica, "create single", "error: topic `single' exists")

        adminForReplica(0, "create single", ["ConnectionRefused", "ConnectFailed"])

        startReplica(0)

        adminForReplica(0, "create single", "error: topic `single' exists")
        current.writeln("ok")

        self.runadmin(current, "destroy single")

        current.write("testing topic creation without master... ")
        sys.stdout.flush()

        stopReplica(2)

        self.runadmin(current, "create single")

        for replica in range(0, 2):
            adminForReplica(replica, "create single", "error: topic `single' exists")

        adminForReplica(2, "create single", ["ConnectionRefused", "ConnectFailed"])

        startReplica(2)

        adminForReplica(2, "create single", "error: topic `single' exists")
        current.writeln("ok")

        # All replicas are running

        current.write("testing topic destruction without replica... ")

        stopReplica(0)

        self.runadmin(current, "destroy single")

        for replica in range(1, 3):
            adminForReplica(replica, "destroy single", "error: couldn't find topic `single'")

        adminForReplica(0, "destroy single", ["ConnectionRefused", "ConnectFailed"])

        startReplica(0)

        adminForReplica(0, "destroy single", "error: couldn't find topic `single'")
        current.writeln("ok")

        current.write("testing topic destruction without master... ")
        sys.stdout.flush()

        self.runadmin(current, "create single")
        stopReplica(2)

        self.runadmin(current, "destroy single")

        for replica in range(0, 2):
            adminForReplica(replica, "destroy single", "error: couldn't find topic `single'")

        adminForReplica(2, "destroy single", ["ConnectionRefused", "ConnectFailed"])

        startReplica(2)

        adminForReplica(2, "destroy single", "error: couldn't find topic `single'")
        current.writeln("ok")

        # Now test subscription/unsubscription on all replicas.

        self.runadmin(current, "create single")

        current.write("testing subscription across replicas... ")
        sys.stdout.flush()
        runsub2()

        for replica in range(0, 3):
            runsub2(replica, "IceStorm::AlreadySubscribed")
        current.writeln("ok")

        current.write("testing unsubscription across replicas... ")
        sys.stdout.flush()
        rununsub2()

        for replica in range(0, 3):
            rununsub2(replica)
        current.writeln("ok")

        current.write("testing subscription without master... ")
        sys.stdout.flush()
        stopReplica(2)

        runsub2()

        for replica in range(0, 2):
            runsub2(replica, "IceStorm::AlreadySubscribed")

        runsub2(2, ["ConnectionRefused", "ConnectFailed"])

        startReplica(2)

        runsub2(2, "IceStorm::AlreadySubscribed")
        current.writeln("ok")

        current.write("testing unsubscription without master... ")
        sys.stdout.flush()
        stopReplica(2)

        rununsub2()

        for replica in range(0, 2):
            rununsub2(replica)

        rununsub2(2, ["ConnectionRefused", "ConnectFailed"])

        startReplica(2)

        rununsub2(2)
        current.writeln("ok")

        current.write("testing subscription without replica... ")
        sys.stdout.flush()
        stopReplica(0)

        runsub2()

        for replica in range(1, 3):
            runsub2(replica, "IceStorm::AlreadySubscribed")

        runsub2(0, ["ConnectionRefused", "ConnectFailed"])

        startReplica(0)

        runsub2(0, "IceStorm::AlreadySubscribed")
        current.writeln("ok")

        current.write("testing unsubscription without replica... ")
        stopReplica(0)

        rununsub2()

        for replica in range(1, 3):
            rununsub2(replica)

        rununsub2(0, ["ConnectionRefused", "ConnectFailed"])

        startReplica(0)

        rununsub2(0)
        current.writeln("ok")

        # All replicas are running

        current.write("running twoway subscription test... ")
        runtest("--twoway")
        current.writeln("ok")

        current.write("running ordered subscription test... ")
        runtest("--ordered")
        current.writeln("ok")

        stopReplica(2)

        current.write("running twoway, ordered subscription test without master... ")
        runtest("--twoway")
        runtest("--ordered")
        current.writeln("ok")

        startReplica(2)
        stopReplica(0)

        current.write("running twoway, ordered subscription test without replica... ")
        runtest("--twoway")
        runtest("--ordered")
        current.writeln("ok")

        startReplica(0)

        current.write("running cycle publishing test... ")
        sys.stdout.flush()
        runtest("--twoway", "--cycle")
        current.writeln("ok")

        current.write("stopping replicas... ")
        sys.stdout.flush()
        self.stopIceStorm(current)
        current.writeln("ok")

TestSuite(__file__, [ IceStormRep1TestCase("replicated", icestorm=icestorm) ], multihost=False)
