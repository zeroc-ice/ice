# Copyright (c) ZeroC, Inc.


from __future__ import annotations

import os

from IceBoxUtil import IceBox
from IceGridUtil import IceGridClient, IceGridNode, IceGridTestCase
from Util import Driver, Process, Props, TestSuite, Windows, platform


class IceGridUpdateTestCase(IceGridTestCase):
    def setupClientSide(self, current: Driver.Current) -> None:
        IceGridTestCase.setupClientSide(self, current)
        current.mkdirs("db/node-1")
        current.mkdirs("db/node-2")


def clientProps(process: Process, current: Driver.Current) -> Props:
    testcase = current.getTestCase()
    assert isinstance(testcase, IceGridTestCase)
    return {
        "NodePropertiesOverride": testcase.icegridnode[0].getPropertiesOverride(current),
        "IceBoxExe": IceBox().getCommandLine(current),
        "IceGridNodeExe": IceGridNode().getCommandLine(current),
        "ServerDir": current.getBuildDir("server"),
        "TestDir": "{testdir}",
    }


if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(
        __file__,
        [IceGridUpdateTestCase(application=None, client=IceGridClient(props=clientProps))],
        multihost=False,
    )
