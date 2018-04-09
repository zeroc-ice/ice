# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

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

if os.getuid() != 0:
    TestSuite(__file__,
              [IceGridNoRestartUpdateTestCase(application=None, client=IceGridClient(props=clientProps))],
              multihost=False)
