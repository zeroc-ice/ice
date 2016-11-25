# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

clientProps = lambda process, current: {
    "IceDir" : toplevel,
    "ServerDir" : current.getBuildDir("server"),
}

TestSuite(__file__, [ IceGridTestCase(client=IceGridClient(props=clientProps)) ], runOnMainThread=True, multihost=False)