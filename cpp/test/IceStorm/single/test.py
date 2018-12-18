# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

#
# Make sure the subscriber uses a larger size receive buffer size then
# the IceStorm send buffer size. This ensures the test works with bogus
# OS configurations where the reicever buffer size is smaller than the
# send buffer size (causing the received messages to be truncated). See
# bug #6070 and #7558.
#
props = { "Ice.UDP.SndSize" : 2048 * 1024, "Ice.Warn.Dispatch" : 0 }
persistent = IceStorm(props = props)
transient = IceStorm(props = props, transient=True)
replicated = [ IceStorm(replica=i, nreplicas=3, props = props) for i in range(0,3) ]

sub = Subscriber(args=["{testcase.parent.name}"], props = { "Ice.UDP.RcvSize" : 4096 * 1024 }, readyCount=3)
pub = Publisher(args=["{testcase.parent.name}"])

class IceStormSingleTestCase(IceStormTestCase):

    def setupClientSide(self, current):
        self.runadmin(current, "create single")

    def teardownClientSide(self, current, success):
        self.runadmin(current, "destroy single")
        self.shutdown(current)

TestSuite(__file__, [
    IceStormSingleTestCase("persistent", icestorm=persistent, client=ClientServerTestCase(client=pub, server=sub)),
    IceStormSingleTestCase("transient", icestorm=transient, client=ClientServerTestCase(client=pub, server=sub)),
    IceStormSingleTestCase("replicated", icestorm=replicated, client=ClientServerTestCase(client=pub, server=sub)),
], multihost=False)
