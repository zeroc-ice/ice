# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

class IceGridNoRestartUpdateTestCase(IceGridTestCase):

    def setupClientSide(self, current):
        IceGridTestCase.setupClientSide(self, current)
        current.mkdirs("db/node1")
        current.mkdirs("db/node2")

clientProps = lambda process, current: {
    "IceBoxExe" : IceBox().getCommandLine(current),
    "ServerDir" : current.getBuildDir("server"),
    "ServiceDir" : current.getBuildDir("testservice")
}

if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(__file__,
              [IceGridNoRestartUpdateTestCase(application=None, client=IceGridClient(props=clientProps))],
              multihost=False)
