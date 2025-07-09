# -*- coding: utf-8 -*-
# Copyright (c) ZeroC, Inc.

from Util import ClientServerTestCase, TestSuite

# Enable some tracing to allow investigating test failures
traceProps = {"Ice.Trace.Network": 3, "Ice.Trace.Protocol": 1}

TestSuite(__name__, [ClientServerTestCase(traceProps=traceProps)])
