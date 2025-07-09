# Copyright (c) ZeroC, Inc.


import os

from IceBoxUtil import IceBox
from IceGridUtil import IceGridClient, IceGridTestCase
from Util import TestSuite, Windows, platform


class IceGridNoRestartUpdateTestCase(IceGridTestCase):
    def setupClientSide(self, current):
        IceGridTestCase.setupClientSide(self, current)
        current.mkdirs("db/node1")
        current.mkdirs("db/node2")


def clientProps(process, current):
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
