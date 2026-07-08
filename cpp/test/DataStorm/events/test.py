# Copyright (c) ZeroC, Inc.

from DataStormUtil import Reader, Writer
from Util import ClientServerTestCase, TestSuite

traceProps = {
    "DataStorm.Trace.Topic": 1,
    "DataStorm.Trace.Session": 3,
    "DataStorm.Trace.Data": 2,
}

multicastProps = {"DataStorm.Node.Multicast.Enabled": 1}

# Force the legacy protocol epoch 0 on both nodes to exercise the receiver-side initSamples initialization path used
# with Ice 3.8.0-3.8.2 peers, in addition to the default run, which negotiates epoch 1 and uses initializeReaders.
epoch0Props = {"DataStorm.Node.MaxProtocolEpoch": 0}

test_cases = [
    ClientServerTestCase(name="Writer/Reader", client=Writer(), server=Reader(), traceProps=traceProps),
]

test_cases.append(
    ClientServerTestCase(
        name="Writer/Reader multicast enabled",
        client=Writer(props=multicastProps),
        server=Reader(props=multicastProps),
        traceProps=traceProps,
    ),
)

test_cases.append(
    ClientServerTestCase(
        name="Writer/Reader protocol epoch 0",
        client=Writer(props=epoch0Props),
        server=Reader(props=epoch0Props),
        traceProps=traceProps,
    ),
)

TestSuite(__file__, test_cases)
