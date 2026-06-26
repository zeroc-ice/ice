# Copyright (c) ZeroC, Inc.

#
# Coverage test for DataStorm session reconnection with the writer ACTIVE during the reconnect.
#
# The existing reliability test forces reconnects only after the writer has published its whole batch and gone
# idle, so the reader always resyncs from retained history. Here the writer publishes a known sequence at a steady
# pace while the reader repeatedly closes the session connection, so live `s()` sample forwards overlap the resync.
# The reader asserts it receives every sample exactly once and in order across all the reconnects.
#

from DataStormUtil import Reader, Writer
from Util import ClientServerTestCase, TestSuite

traceProps = {
    "DataStorm.Trace.Topic": 1,
    "DataStorm.Trace.Session": 3,
    "DataStorm.Trace.Data": 2,
}

# Reader connects (client) to the writer, which owns the public endpoint (server).
readerProps = {
    "DataStorm.Node.Multicast.Enabled": 0,
    "DataStorm.Node.Server.Enabled": 0,
    "DataStorm.Node.ConnectTo": "tcp -p {port1}",
    "DataStorm.Node.Name": "reader-app",
}

writerProps = {
    "DataStorm.Node.Multicast.Enabled": 0,
    "DataStorm.Node.Server.Endpoints": "tcp -p {port1}",
    "DataStorm.Node.ConnectTo": "",
    "DataStorm.Node.Name": "writer-app",
}

TestSuite(
    __file__,
    [
        ClientServerTestCase(
            name="reader reconnect while writer is actively publishing",
            client=Reader(props=readerProps),
            server=Writer(props=writerProps),
            traceProps=traceProps,
        )
    ],
)
