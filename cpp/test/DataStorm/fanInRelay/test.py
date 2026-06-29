# Copyright (c) ZeroC, Inc.

#
# Ensures that multiple subscribers which share the same subscriber session id, but originate in different nodes,
# are all notified when the publisher they are subscribed to disconnects.
#
# Two subscriber applications and one publisher application connect to a single relay node. The publisher has no
# public endpoint, so the subscribers reach it through the relay. The publisher is then terminated, dropping the
# relay's connection to it; both subscribers must observe that there are no more writers.
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

        # Wait until both subscribers have attached to the publisher through the relay.
        self.publisher.expect(current, "writer published", timeout=30)
        self.subA.expect(current, "reader attached", timeout=30)
        self.subB.expect(current, "reader attached", timeout=30)

        # Terminate the publisher, dropping the relay's connection to it.
        current.processes[self.publisher].terminate()

        # Both subscribers must observe that the publisher is gone.
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
            name="fan-in subscribers notified of publisher disconnect",
            relayNode=Node(desc="relay", props=relay("relay-node")),
            publisher=Writer(props=app("publisher-app")),
            subA=Reader(props=app("subscriber-a")),
            subB=Reader(props=app("subscriber-b")),
        ),
    ],
)
