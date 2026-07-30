# Copyright (c) ZeroC, Inc.


from __future__ import annotations

import os

from IceGridUtil import IceGridClient, IceGridTestCase
from Util import Driver, Process, Props, TestSuite, Windows, platform


def clientProps(process: Process, current: Driver.Current) -> Props:
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
        [IceGridTestCase(client=IceGridClient(props=clientProps, traceProps=traceProps))],
        runOnMainThread=True,
        multihost=False,
    )
