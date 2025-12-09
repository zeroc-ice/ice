# Copyright (c) ZeroC, Inc.

from IceStormUtil import IceStorm, IceStormTestCase
from Util import Client, TestSuite

props = {}
persistent = IceStorm(props=props)
transient = IceStorm(props=props, transient=True)
replicated = [IceStorm(replica=i, nreplicas=3, props=props) for i in range(0, 3)]


class CreateOrRetrieveTestCase(IceStormTestCase):
    def setupClientSide(self, current):
        # No setup needed - test creates and destroys its own topics
        pass

    def teardownClientSide(self, current, success):
        self.shutdown(current)


TestSuite(
    __file__,
    [
        CreateOrRetrieveTestCase("persistent", icestorm=persistent, client=Client()),
        CreateOrRetrieveTestCase("transient", icestorm=transient, client=Client()),
        CreateOrRetrieveTestCase("replicated", icestorm=replicated, client=Client()),
    ],
    multihost=False,
)
