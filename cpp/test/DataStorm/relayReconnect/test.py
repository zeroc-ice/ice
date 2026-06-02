# Copyright (c) ZeroC, Inc.

#
# Regression test for topic announcement propagation across a DataStorm relay reconnection.
#
# A writer and a reader are connected to opposite ends of a relay chain, and the middle relay is started last - after
# both applications have already delivered their topic announcements to the relays on each side. For the writer and
# reader to discover each other, the relays must replay the announcements they received earlier across the session to
# the middle relay once it comes up. The test verifies that discovery completes and a sample flows end to end; without
# that replay the announcements never cross the middle relay and the applications would wait forever for discovery.
#
# Each case makes the failure deterministic by starting the middle relay only after both announcements have reached the
# relays that must replay them (the test synchronizes on the relays' "topic ... announced" trace markers, written to
# per-relay log files). Two topologies are covered:
#
#   "topic propagation" - each application is connected directly to a relay:
#       writer-app -> relay-node-1 -> relay-node-2 -> relay-node-3 <- reader-app
#                                     (middle, started last)
#
#   "interior relay" - the announcements are forwarded to interior relays, so the announced node is the application,
#   not the adjacent relay the interior relay has a session with:
#       writer-app -> node1 -> node2 -> nodeB -> node3 -> node4 <- reader-app
#                              (interior) (last) (interior)
#

import os

from DataStormUtil import Node, Reader, Writer, waitForLogMessage
from Util import ClientServerTestCase, TestSuite

# Constant, fast retry so the relays reconnect promptly once the middle relay starts. Session trace level 2+ makes the
# relays log the "topic ... announced" markers; each relay's trace goes to its own Ice.LogFile (set in runClientSide)
# to keep the test console clean.
nodeProps = {
    "DataStorm.Node.Multicast.Enabled": 0,
    "DataStorm.Node.RetryDelay": 200,
    "DataStorm.Node.RetryMultiplier": 1,
    "DataStorm.Trace.Session": 3,
}


def relay(endpoint, connectTo, name):
    return dict(
        nodeProps,
        **{
            "DataStorm.Node.Server.Endpoints": f"tcp -p {{port{endpoint}}}",
            "DataStorm.Node.ConnectTo": f"tcp -p {{port{connectTo}}}" if connectTo else "",
            "DataStorm.Node.Name": name,
        },
    )


def app(connectTo, name):
    return {
        "DataStorm.Node.Multicast.Enabled": 0,
        "DataStorm.Node.Server.Enabled": 0,
        "DataStorm.Node.ConnectTo": f"tcp -p {{port{connectTo}}}",
        "DataStorm.Node.Name": name,
    }


class RelayReconnectTestCase(ClientServerTestCase):
    # nodes: the relays in the chain.
    # middleRelay: the relay that joins the two halves, started last.
    # writerRelay/readerRelay: the relays whose logs confirm the writer/reader announcement arrived before the middle
    # relay starts.
    def __init__(self, nodes, middleRelay, writerRelay, readerRelay, reader, writer, **kwargs):
        ClientServerTestCase.__init__(self, **kwargs)
        self.nodes = nodes
        self.middleRelay = middleRelay
        self.writerRelay = writerRelay
        self.readerRelay = readerRelay
        self.reader = reader
        self.writer = writer

    # The applications and relays are managed explicitly in runClientSide, so suppress the default "client"/"server"
    # processes the framework would otherwise instantiate.
    def getClientType(self):
        return None

    def getServerType(self):
        return None

    def runClientSide(self, current):
        # Send each relay's trace to its own log file (removing any stale copy) so the test can synchronize on the
        # announcement markers without polluting the test console.
        self.logs = {}
        for node in self.nodes:
            log = os.path.join(current.testsuite.getPath(), f"{node.desc}.log")
            if os.path.exists(log):
                os.remove(log)
            node.props["Ice.LogFile"] = log
            self.logs[node] = log

        # Bring up every relay except the middle one, then both applications. Each application announces its topic to
        # the relay it connects to; with the middle relay down, those announcements reach the relays on each side but
        # cannot cross to the other.
        for node in self.nodes:
            if node is not self.middleRelay:
                node.start(current)
        self.writer.start(current)
        self.reader.start(current)

        # Wait until each announcement has reached the relay that must later replay it across the middle relay. This
        # removes the timing window: the middle relay is started only once both announcements are in place.
        waitForLogMessage(self.logs[self.writerRelay], "topic writer.*announced.*writer-app")
        waitForLogMessage(self.logs[self.readerRelay], "topic reader.*announced.*reader-app")

        # Bring up the middle relay last. A relay that does not replay the announcements it received earlier would
        # never propagate them across the new sessions, and discovery would deadlock.
        self.middleRelay.start(current)

        self.writer.expect(current, "writer completed", timeout=30)
        self.reader.expect(current, "reader completed", timeout=30)

        self.writer.stop(current, waitSuccess=True)
        self.reader.stop(current, waitSuccess=True)

    def teardownClientSide(self, current, success):
        for process in [self.writer, self.reader] + list(reversed(self.nodes)):
            if process.isStarted(current):
                process.stop(current)

        # On success, remove the per-relay log files; keep them on failure for diagnostics.
        if success:
            for log in getattr(self, "logs", {}).values():
                if os.path.exists(log):
                    os.remove(log)


# Topology 1: writer on the head relay, reader on the tail relay, middle relay (node2) started last.
chain = [
    Node(desc="node1", props=relay(1, 2, "relay-node-1")),
    Node(desc="node2", props=relay(2, 3, "relay-node-2")),
    Node(desc="node3", props=relay(3, None, "relay-node-3")),
]

# Topology 2: the writer and reader announcements are forwarded to the interior relays node2 and node3 before the
# middle relay (nodeB) is started.
interior = [
    Node(desc="node1", props=relay(1, 2, "node1")),
    Node(desc="node2", props=relay(2, 3, "node2")),
    Node(desc="nodeB", props=relay(3, 4, "nodeB")),
    Node(desc="node3", props=relay(4, 5, "node3")),
    Node(desc="node4", props=relay(5, None, "node4")),
]

TestSuite(
    __file__,
    [
        RelayReconnectTestCase(
            name="relay reconnect topic propagation",
            nodes=chain,
            middleRelay=chain[1],  # node2
            writerRelay=chain[0],  # node1 receives the writer announcement directly
            readerRelay=chain[2],  # node3 receives the reader announcement directly
            reader=Reader(props=app(3, "reader-app")),
            writer=Writer(props=app(1, "writer-app")),
        ),
        RelayReconnectTestCase(
            name="interior relay forwarded announcement replay",
            nodes=interior,
            middleRelay=interior[2],  # nodeB
            writerRelay=interior[1],  # node2 receives the forwarded writer announcement
            readerRelay=interior[3],  # node3 receives the forwarded reader announcement
            reader=Reader(props=app(5, "reader-app")),
            writer=Writer(props=app(1, "writer-app")),
        ),
    ],
)
