# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

# Enable some tracing to allow investigating test failures
traceProps = {
    "Ice.Trace.Network" : 2,
    "Ice.Trace.Retry" : 1,
    "Ice.Trace.Protocol" : 1
}

TestSuite(__name__, [ClientServerTestCase(traceProps=traceProps)])
