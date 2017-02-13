# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

clientProps=lambda process, current: { "TestDir" : current.getBuildDir("server") }

TestSuite(__file__, [
    IceGridTestCase("without targets",
                    icegridnode=IceGridNode(envs={ "MY_FOO" : 12 }),
                    client=IceGridClient(props=clientProps)),
    IceGridTestCase("with targets",
                    icegridnode=IceGridNode(envs={ "MY_FOO" : 12 }),
                    client=IceGridClient(props=clientProps),
                    targets=["moreservers", "moreservices", "moreproperties"])
], libDirs=["testservice"], multihost=False)
