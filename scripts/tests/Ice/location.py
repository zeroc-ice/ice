# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

# Enable some tracing to allow investigating test failures
traceProps = {
    "Ice.Trace.Locator" : 2,
    "Ice.Trace.Protocol" : 1
}

TestSuite(__name__, [ClientServerTestCase(traceProps=traceProps)])
