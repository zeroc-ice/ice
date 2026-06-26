# Copyright (c) ZeroC, Inc.

#
# Regression test for a DataStorm fan-in relay session-identity collision.
#
# Two subscriber applications and one publisher application all connect to a single relay node. The publisher
# has no public endpoint, so the subscribers reach it through the relay. Each fresh subscriber node assigns
# its first subscriber session the same Ice identity {"1","s"} (the session-id counter is per-node and has no
# node-unique component), and the relay tracks relayed sessions in a map keyed by that identity. The second
# subscriber's session therefore overwrites the first's in the relay's map.
#
# When the relay loses its connection to the publisher, it notifies only the surviving session, so the
# overwritten subscriber is never told the publisher disconnected and waitForNoWriters() blocks forever.
#
#   subA --app--> \
#                  relay  <--app-- publisher   (publisher and subscribers have no public endpoint)
#   subB --app--> /
#

from DataStormUtil import Node, Reader, Writer
from Util import ClientServerTestCase, TestSuite

nodeProps = {
    "DataStorm.Node.Multicast.Enabled": 0,
    "DataStorm.Trace.Session": 2,
}


def relay(name):
    return dict(
        nodeProps,
        **{
            "DataStorm.Node.Server.Endpoints": "tcp -p {port1}",
            "DataStorm.Node.ConnectTo": "",
            "DataStorm.Node.Name": name,
        },
    )


def app(name):
    return {
        "DataStorm.Node.Multicast.Enabled": 0,
        "DataStorm.Node.Server.Enabled": 0,
        "DataStorm.Node.ConnectTo": "tcp -p {port1}",
        "DataStorm.Node.Name": name,
    }


class FanInTestCase(ClientServerTestCase):
    def __init__(self, relayNode, publisher, subA, subB, **kwargs):
        ClientServerTestCase.__init__(self, **kwargs)
        self.relayNode = relayNode
        self.publisher = publisher
        self.subA = subA
        self.subB = subB

    # The processes are managed explicitly in runClientSide, so suppress the default client/server.
    def getClientType(self):
        return None

    def getServerType(self):
        return None

    def runClientSide(self, current):
        self.relayNode.start(current)
        self.publisher.start(current)
        self.subA.start(current)
        self.subB.start(current)

        # Wait until both subscribers have attached to the publisher through the relay, so both subscriber
        # sessions -- with their colliding identities -- are registered in the relay's session map.
        self.publisher.expect(current, "writer published", timeout=30)
        self.subA.expect(current, "reader attached", timeout=30)
        self.subB.expect(current, "reader attached", timeout=30)

        # Drop the relay's connection to the publisher by terminating it. The relay then notifies the relayed
        # subscriber sessions of the disconnection.
        current.processes[self.publisher].terminate()

        # Both subscribers must observe the disconnect. Without the fix, the overwritten subscriber's session
        # was dropped from the relay's map, so it is never notified and this hangs.
        self.subA.expect(current, "reader saw no writers", timeout=15)
        self.subB.expect(current, "reader saw no writers", timeout=15)

        self.subA.stop(current, waitSuccess=True)
        self.subB.stop(current, waitSuccess=True)

    def teardownClientSide(self, current, success):
        for process in [self.subA, self.subB, self.publisher, self.relayNode]:
            if process.isStarted(current):
                process.stop(current)


TestSuite(
    __file__,
    [
        FanInTestCase(
            name="fan-in relay session-identity collision",
            relayNode=Node(desc="relay", props=relay("relay-node")),
            publisher=Writer(props=app("publisher-app")),
            subA=Reader(props=app("subscriber-a")),
            subB=Reader(props=app("subscriber-b")),
        ),
    ],
)
