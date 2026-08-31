# Copyright (c) ZeroC, Inc.

#
# A peer that speaks the DataStorm session protocol directly drives session creation with a real DataStorm node,
# controlling when each confirmCreateSession is answered. Two handshakes are outstanding at once - which a
# two-relay topology produces on its own, since every reconnection attempt can take a different route - so the
# handshake the node acts on is not the one that is still current.
#

from typing import Any

from DataStormUtil import DataStormProcess
from Util import Client, ClientServerTestCase, Server, TestSuite

traceProps = {
    "DataStorm.Trace.Topic": 1,
    "DataStorm.Trace.Session": 3,
    "DataStorm.Trace.Data": 2,
}


class Peer(Client, DataStormProcess):
    processType = "client"


class NodeUnderTest(Server, DataStormProcess):
    processType = "writer"

    def __init__(self, *args: Any, **kargs: Any):
        Server.__init__(self, *args, waitForShutdown=False, readyCount=0, **kargs)


nodeProps = {
    "DataStorm.Node.Multicast.Enabled": 0,
    "DataStorm.Node.Server.Endpoints": "tcp -p {port1}",
    "DataStorm.Node.ConnectTo": "",
    "DataStorm.Node.Name": "node-under-test",
    # A session parked for its peer is removed after getRetryDelay(RetryCount) * 2, which is 128 times this
    # value. Keeping it small lets the test observe that removal instead of waiting a minute for it.
    "DataStorm.Node.RetryDelay": 10,
}

TestSuite(
    __file__,
    [ClientServerTestCase(client=Peer(), server=NodeUnderTest(props=nodeProps), traceProps=traceProps)],
)
