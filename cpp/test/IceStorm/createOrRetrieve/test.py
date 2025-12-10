# Copyright (c) ZeroC, Inc.

from IceStormUtil import IceStorm, IceStormProcess, IceStormTestCase
from Util import Client, ClientTestCase, TestSuite

props = {}
persistent = IceStorm(props=props)
transient = IceStorm(props=props, transient=True)
replicated = [IceStorm(replica=i, nreplicas=3, props=props) for i in range(0, 3)]


class CreateOrRetrieveTestCase(IceStormTestCase):
    def teardownClientSide(self, current, success):
        self.shutdown(current)


class IceStormClient(IceStormProcess, Client):
    processType = "client"

    def __init__(self, instanceName=None, instance=None, *args, **kargs):
        Client.__init__(self, *args, **kargs)
        IceStormProcess.__init__(self, instanceName, instance)

    getParentProps = Client.getProps  # Used by IceStormProcess to get the client properties


TestSuite(
    __file__,
    [
        CreateOrRetrieveTestCase("persistent", icestorm=persistent, client=ClientTestCase(client=IceStormClient())),
        CreateOrRetrieveTestCase("transient", icestorm=transient, client=ClientTestCase(client=IceStormClient())),
        CreateOrRetrieveTestCase("replicated", icestorm=replicated, client=ClientTestCase(client=IceStormClient())),
    ],
    multihost=False,
)
