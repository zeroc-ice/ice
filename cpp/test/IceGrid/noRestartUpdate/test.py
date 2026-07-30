# Copyright (c) ZeroC, Inc.


from __future__ import annotations

import os

from IceBoxUtil import IceBox
from IceGridUtil import IceGridClient, IceGridTestCase
from Util import Driver, Process, Props, TestSuite, Windows, platform


class IceGridNoRestartUpdateTestCase(IceGridTestCase):
    def setupClientSide(self, current: Driver.Current) -> None:
        IceGridTestCase.setupClientSide(self, current)
        current.mkdirs("db/node1")
        current.mkdirs("db/node2")


def clientProps(process: Process, current: Driver.Current) -> Props:
    return {
        "IceBoxExe": IceBox().getCommandLine(current),
        "ServerDir": current.getBuildDir("server"),
        "ServiceDir": current.getBuildDir("testservice"),
    }


if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(
        __file__,
        [IceGridNoRestartUpdateTestCase(application=None, client=IceGridClient(props=clientProps))],
        multihost=False,
    )
