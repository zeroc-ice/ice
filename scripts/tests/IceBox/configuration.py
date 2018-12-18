# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

TestSuite(__name__, [
    ClientServerTestCase("client/server #1", server=IceBox("{testdir}/config.icebox")),
    ClientServerTestCase("client/server #2", server=IceBox("{testdir}/config.icebox2"))
], libDirs=["testservice"], runOnMainThread=True, options={ "protocol" : ["tcp"], "ipv6" : [False], "mx" : [False] }, multihost=False)
