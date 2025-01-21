# Copyright (c) ZeroC, Inc.


import os
from IceGridUtil import IceGridClient, IceGridTestCase
from Util import TestSuite, Windows, platform


def clientProps(process, current):
    return {"ServerDir": current.getBuildDir("server")}


# Enable some tracing to allow investigating test failures
traceProps = {
    "Ice.Trace.Network": 2,
    "Ice.Trace.Retry": 1,
    "Ice.Trace.Protocol": 1,
}

if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(
        __file__,
        [
            IceGridTestCase(
                client=IceGridClient(props=clientProps, traceProps=traceProps)
            )
        ],
        runOnMainThread=True,
        multihost=False,
    )
