# Copyright (c) ZeroC, Inc.

from __future__ import annotations

from typing import Any

from IceStormUtil import IceStorm, IceStormProcess, IceStormTestCase
from Util import Client, ClientTestCase, Driver, Props, TestSuite

props = {}
persistent = IceStorm(props=props)
transient = IceStorm(props=props, transient=True)
replicated = [IceStorm(replica=i, nreplicas=3, props=props) for i in range(0, 3)]


class CreateOrRetrieveTestCase(IceStormTestCase):
    def teardownClientSide(self, current: Driver.Current, success: bool) -> None:
        self.shutdown(current)


class IceStormClient(IceStormProcess, Client):
    processType = "client"

    def __init__(self, instanceName: str | None = None, instance: IceStorm | None = None, *args: Any, **kargs: Any):
        Client.__init__(self, *args, **kargs)
        IceStormProcess.__init__(self, instanceName, instance)

    def getParentProps(self, current: Driver.Current) -> Props:
        # IceStormProcess.getProps calls this to reach the Client props rather than its own entry
        # in the MRO.
        return Client.getProps(self, current)


TestSuite(
    __file__,
    [
        CreateOrRetrieveTestCase("persistent", icestorm=persistent, client=ClientTestCase(client=IceStormClient())),
        CreateOrRetrieveTestCase("transient", icestorm=transient, client=ClientTestCase(client=IceStormClient())),
        CreateOrRetrieveTestCase("replicated", icestorm=replicated, client=ClientTestCase(client=IceStormClient())),
    ],
    multihost=False,
)
