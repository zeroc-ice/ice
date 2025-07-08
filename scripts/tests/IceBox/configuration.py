# Copyright (c) ZeroC, Inc.

from IceBoxUtil import IceBox
from Util import ClientServerTestCase, TestSuite


TestSuite(
    __name__,
    [
        # readyCount=2 for Ice.Admin and the "bundleName" from IceBox.PrintServicesReady
        ClientServerTestCase("client/server #1", server=IceBox("{testdir}/config.icebox", readyCount=2)),
        ClientServerTestCase("client/server #2", server=IceBox("{testdir}/config.icebox2", readyCount=2)),
    ],
    libDirs=["testservice"],
    runOnMainThread=True,
    options={"protocol": ["tcp"], "ipv6": [False], "mx": [False]},
    multihost=False,
)
