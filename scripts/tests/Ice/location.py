# Copyright (c) ZeroC, Inc.

# Enable some tracing to allow investigating test failures
from Util import ClientServerTestCase, TestSuite


traceProps = {"Ice.Trace.Locator": 2, "Ice.Trace.Protocol": 1}

TestSuite(__name__, [ClientServerTestCase(traceProps=traceProps)])
