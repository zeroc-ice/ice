# Copyright (c) ZeroC, Inc.

#
# A reader that already holds a partial-update base reconnects across a gap in the writer's history.
#
# The writer keeps no history (sampleCount = 0) and publishes a full value plus a partial update while the session is
# down. Resuming from the reader's last sample id cannot replay them, so the writer re-seeds the reader from its
# current per-key base instead, delivered as a single full update carrying the resolved value. The scenarios in
# DataStorm/partial cover the same re-seeding for readers that join fresh, with a last sample id of 0.
#
# The writer opens the gap by closing the session connection and publishing immediately afterwards, which is only
# reliable while the two nodes share a single connection. This test therefore pins the topology: multicast is disabled
# and the writer runs as a client with no server endpoint, so the reader can never connect back to it. Given more than
# one connection between the nodes - multicast, or a relay chain - DataStorm can route the writer's publisher session
# and the barrier's subscriber session over different connections, and closing one would leave the other delivering.
#

from DataStormUtil import Reader, Writer
from Util import ClientServerTestCase, TestSuite

traceProps = {
    "DataStorm.Trace.Topic": 1,
    "DataStorm.Trace.Session": 3,
    "DataStorm.Trace.Data": 2,
}

# The reader listens and never connects out; the writer only connects in. One connection carries every session between
# them, so closing it interrupts the writer's data session.
readerProps = {
    "DataStorm.Node.Multicast.Enabled": 0,
    "DataStorm.Node.Server.Endpoints": "tcp -p {port1}",
    "DataStorm.Node.ConnectTo": "",
}

writerProps = {
    "DataStorm.Node.Multicast.Enabled": 0,
    "DataStorm.Node.Server.Enabled": 0,
    "DataStorm.Node.ConnectTo": "tcp -p {port1}",
}

TestSuite(
    __file__,
    [
        ClientServerTestCase(
            name="partial update after reconnect across a history gap",
            client=Writer(props=writerProps),
            server=Reader(props=readerProps),
            traceProps=traceProps,
        )
    ],
)
