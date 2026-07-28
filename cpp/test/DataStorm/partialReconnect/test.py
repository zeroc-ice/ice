# Copyright (c) ZeroC, Inc.

#
# A reader that already holds a partial-update base reconnects across a gap in the writer's history.
#
# The writer keeps no history (sampleCount = 0) and publishes a full value plus a partial update while the session is
# down. Resuming from the reader's last sample id cannot replay them, so the writer re-seeds the reader from its
# current per-key base instead, delivered as a single full update carrying the resolved value. The scenarios in
# DataStorm/partial cover the same re-seeding for readers that join fresh, with a last sample id of 0.
#
# The writer opens the gap by holding its node adapter and closing the session connection, then publishes and
# activates the adapter again. A held adapter stops reading requests, so the reader cannot re-establish the session
# while the samples are published. This requires the writer to be the node that listens, because only the node whose
# adapter is held can decide when the gap ends: the barrier that would otherwise carry that decision runs over the
# session being interrupted.
#
# The topology is pinned for the same reason the gap is: multicast is disabled and the reader has no server endpoint,
# so a single connection carries every session between the two nodes. Given more than one connection - multicast, or a
# relay chain - DataStorm can route the writer's publisher session and the barrier's subscriber session over different
# connections, and closing one would leave the other delivering.
#

from DataStormUtil import Reader, Writer
from Util import ClientServerTestCase, TestSuite

traceProps = {
    "DataStorm.Trace.Topic": 1,
    "DataStorm.Trace.Session": 3,
    "DataStorm.Trace.Data": 2,
}

# The writer listens and never connects out; the reader only connects in. One connection carries every session between
# them, so closing it interrupts the writer's data session.
writerProps = {
    "DataStorm.Node.Multicast.Enabled": 0,
    "DataStorm.Node.Server.Endpoints": "tcp -p {port1}",
    "DataStorm.Node.ConnectTo": "",
}

readerProps = {
    "DataStorm.Node.Multicast.Enabled": 0,
    "DataStorm.Node.Server.Enabled": 0,
    "DataStorm.Node.ConnectTo": "tcp -p {port1}",
}

TestSuite(
    __file__,
    [
        ClientServerTestCase(
            name="partial update after reconnect across a history gap",
            client=Reader(props=readerProps),
            server=Writer(props=writerProps),
            traceProps=traceProps,
        )
    ],
)
