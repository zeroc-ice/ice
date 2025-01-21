# Copyright (c) ZeroC, Inc.

from DataStormUtil import Reader, Writer
from Util import ClientServerTestCase, TestSuite

traceProps = {
    "DataStorm.Trace.Topic" : 1,
    "DataStorm.Trace.Session" : 3,
    "DataStorm.Trace.Data" : 2,
}

multicastProps = { "DataStorm.Node.Multicast.Enabled" : 1 }

test_cases = [
    ClientServerTestCase(
        name = "Writer/Reader",
        client = Writer(),
        server = Reader(),
        traceProps=traceProps),
]

# Repeat with callbacks running in the custom callback executor.
test_cases.append(
    ClientServerTestCase(
        name = "Writer/Reader with custom callback executor",
        client = Writer(args=["--with-executor"]),
        server = Reader(args=["--with-executor"]),
        traceProps=traceProps),
)

# Repeat with multicast enabled.
test_cases.append(
    ClientServerTestCase(
        name = "Writer/Reader multicast enabled",
        client = Writer(props = multicastProps),
        server = Reader(props = multicastProps),
        traceProps=traceProps),
)

TestSuite(__file__, test_cases)
