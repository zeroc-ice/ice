# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

class IceGridNoRestartUpdateTestCase(IceGridTestCase):

    def setupClientSide(self, current):
        IceGridTestCase.setupClientSide(self, current)
        self.mkdirs("db/node1")
        self.mkdirs("db/node2")

clientProps = lambda process, current: {
    "IceBinDir" : current.testcase.getMapping().getBinDir(None, current),
    "ServerDir" : current.getBuildDir("server"),
    "ServiceDir" : current.getBuildDir("testservice")
}

TestSuite(__file__,
          [IceGridNoRestartUpdateTestCase(application=None, client=IceGridClient(props=clientProps))],
          multihost=False)