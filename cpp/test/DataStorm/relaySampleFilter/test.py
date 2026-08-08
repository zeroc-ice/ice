# Copyright (c) ZeroC, Inc.

#
# A sample-filtered reader reads a writer through a relay node.
#
#   reader-app --app--> relay <--app-- writer-app   (neither application has a public endpoint)
#
# A reader that declares a sample filter is addressed by the writer under a session facet of its own, and the
# reader accepts a live sample only when it arrives under that facet. The relay has to preserve the facet when it
# forwards the invocation, or every live sample the filter matched is discarded on arrival and the reader silently
# stops updating. Initialization is not affected: an initialization batch is routed to the reader element rather
# than to its facet, so the writer publishes only after the reader attached.
#

from __future__ import annotations

from typing import Any

from DataStormUtil import Node, Reader, Writer
from Util import ClientServerTestCase, Driver, Process, Props, TestSuite

commonProps = {
    "DataStorm.Node.Multicast.Enabled": 0,
}


def relay(name: str) -> Props:
    return dict(
        commonProps,
        **{
            "DataStorm.Node.Server.Endpoints": "tcp -p {port1}",
            "DataStorm.Node.ConnectTo": "",
            "DataStorm.Node.Name": name,
        },
    )


def app(name: str) -> Props:
    return dict(
        commonProps,
        **{
            "DataStorm.Node.Server.Enabled": 0,
            "DataStorm.Node.ConnectTo": "tcp -p {port1}",
            "DataStorm.Node.Name": name,
        },
    )


class RelaySampleFilterTestCase(ClientServerTestCase):
    def __init__(self, relayNode: Process, writer: Process, reader: Process, **kwargs: Any):
        ClientServerTestCase.__init__(self, **kwargs)
        self.relayNode = relayNode
        self.writerApp = writer
        self.readerApp = reader

    # The processes are managed explicitly in runClientSide, so suppress the default client/server.
    def getClientType(self):
        return None

    def getServerType(self):
        return None

    def runClientSide(self, current: Driver.Current) -> None:
        self.relayNode.start(current)
        self.writerApp.start(current)
        self.readerApp.start(current)

        self.writerApp.expect(current, "writer published", timeout=30)
        self.readerApp.expect(current, "reader completed", timeout=30)

        self.readerApp.stop(current, waitSuccess=True)
        self.writerApp.expect(current, "writer completed", timeout=30)
        self.writerApp.stop(current, waitSuccess=True)

    def teardownClientSide(self, current: Driver.Current, success: bool) -> None:
        for process in [self.readerApp, self.writerApp, self.relayNode]:
            if process.isStarted(current):
                process.stop(current)


TestSuite(
    __file__,
    [
        RelaySampleFilterTestCase(
            name="sample-filtered reader through a relay",
            relayNode=Node(desc="relay", props=relay("relay-node")),
            writer=Writer(props=app("writer-app")),
            reader=Reader(props=app("reader-app")),
        ),
    ],
)
