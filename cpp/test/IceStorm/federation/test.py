# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

class IceStormFederationTestCase(IceStormTestCase):

    def setupClientSide(self, current):
        self.runadmin(current, "create fed1 fed2 fed3; link fed1 fed2 10; link fed2 fed3 5")

    def runClientSide(self, current):
        current.write("testing oneway subscribers...")
        ClientServerTestCase(client=Publisher(), server=Subscriber()).run(current)
        current.writeln("ok")

        current.write("testing batch subscribers...")
        ClientServerTestCase(client=Publisher(), server=Subscriber(args=["-b"])).run(current)
        current.writeln("ok")

    def teardownClientSide(self, current, success):
        self.runadmin(current, "destroy fed1 fed2 fed3")
        self.shutdown(current)

# Override ReplicatedPublishEndpoints property to empty for testing without replicated publisher
props = {'IceStorm.ReplicatedPublishEndpoints' : ''}

TestSuite(__file__, [
    IceStormFederationTestCase("persistent", icestorm=IceStorm()),
    IceStormFederationTestCase("transient", icestorm=IceStorm(transient=True)),
    IceStormFederationTestCase("replicated with non-replicated publisher",
                               icestorm=[IceStorm(replica=i, nreplicas=3, props=props) for i in range(0,3)]),
    IceStormFederationTestCase("replicated with replicated publisher",
                               icestorm=[IceStorm(replica=i, nreplicas=3) for i in range(0,3)]),
], multihost=False)
