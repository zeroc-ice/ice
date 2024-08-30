# Copyright (c) ZeroC, Inc.

from Util import (
    ClientServerTestCase,
    TestSuite,
)

# Enable some tracing to allow investigating test failures
traceProps = {"Ice.Trace.Network": 2}
testcases = [
    ClientServerTestCase(traceProps=traceProps),
]

TestSuite(
    __name__,
    testcases,
    options={"compress": [False], "protocol": ["ssl", "ws", "tcp"]},
    multihost=False,
)
