# Copyright (c) ZeroC, Inc.

#
# Coverage test for DataStorm relay-to-relay forwarding integrity across a connection drop.
#
# A writer reaches a reader through two chained relay nodes (writer -> relay1 -> relay2 -> reader). No existing
# test ever closes a non-app (relay-to-relay) connection -- the reliability suite only closes app-side
# connections. This single-process test repeatedly drops the relay1<->relay2 hop while the writer is actively
# publishing, and asserts every sample still arrives at the reader exactly once and in order: the relay forwards
# samples losslessly across the hop drops (the relay heals transparently, so the end-to-end session is not even
# disrupted). A regression that dropped or duplicated an in-flight sample on a closed relay hop would fail here.
#

from DataStormUtil import Writer
from Util import ClientTestCase, TestSuite

TestSuite(__file__, [ClientTestCase(client=Writer())])
