# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

TestSuite(__name__, [
    ClientServerTestCase("client/server #1", server=IceBox(args=['--Ice.Config="{testdir}/config.icebox"'])),
    ClientServerTestCase("client/server #2", server=IceBox(args=['--Ice.Config="{testdir}/config.icebox2"']))
], libDirs=["testservice"], runOnMainThread=True, options={ "protocol" : ["tcp"], "ipv6" : [False], "mx" : [False] }, multihost=False)
