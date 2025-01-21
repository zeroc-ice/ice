# -*- coding: utf-8 -*-
# Copyright (c) ZeroC, Inc.

# Enable some tracing to allow investigating test failures
from Util import ClientServerTestCase, TestSuite


traceProps = {"Ice.Trace.Network": 2, "Ice.Trace.Protocol": 1}

TestSuite(__name__, [ClientServerTestCase(traceProps=traceProps)], options={"compress": [False], "serialize": [False]})
