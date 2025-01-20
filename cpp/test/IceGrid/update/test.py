# Copyright (c) ZeroC, Inc.


import os
from IceBoxUtil import IceBox
from IceGridUtil import IceGridClient, IceGridNode, IceGridTestCase
from Util import TestSuite, Windows, platform


class IceGridUpdateTestCase(IceGridTestCase):
    def setupClientSide(self, current):
        IceGridTestCase.setupClientSide(self, current)
        current.mkdirs("db/node-1")
        current.mkdirs("db/node-2")


def clientProps(process, current):
    return {
        "NodePropertiesOverride": current.testcase.icegridnode[0].getPropertiesOverride(
            current
        ),
        "IceBoxExe": IceBox().getCommandLine(current),
        "IceGridNodeExe": IceGridNode().getCommandLine(current),
        "ServerDir": current.getBuildDir("server"),
        "TestDir": "{testdir}",
    }


if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(
        __file__,
        [
            IceGridUpdateTestCase(
                application=None, client=IceGridClient(props=clientProps)
            )
        ],
        multihost=False,
    )
