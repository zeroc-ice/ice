# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

class IceGridUpdateTestCase(IceGridTestCase):

    def setupClientSide(self, current):
        IceGridTestCase.setupClientSide(self, current)
        self.mkdirs("db/node-1")
        self.mkdirs("db/node-2")

clientProps = lambda process, current: {
    "NodePropertiesOverride" : current.testcase.icegridnode[0].getPropertiesOverride(current),
    "IceBinDir" : current.testcase.getMapping().getBinDir(current),
    "ServerDir" : current.getBuildDir("server"),
    "TestDir" : "{testdir}"
}

TestSuite(__file__, [IceGridUpdateTestCase(application=None, client=IceGridClient(props=clientProps))], multihost=False)