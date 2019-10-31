# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

clientProps = lambda process, current: { "ServerDir" : current.getBuildDir("server") }

# Enable some tracing to allow investigating test failures
traceProps = {
    "Ice.Trace.Network" : 2,
    "Ice.Trace.Retry" : 1,
    "Ice.Trace.Protocol" : 1
}

if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(__file__,
              [ IceGridTestCase(client=IceGridClient(props=clientProps, traceProps=traceProps)) ],
              runOnMainThread=True,
              multihost=False)
