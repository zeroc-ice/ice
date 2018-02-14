# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

class IceBoxAdminTestCase(ClientServerTestCase):

    def runClientSide(self, current):

        admin = IceBoxAdmin(args = ['--Ice.Config="{testdir}/config.admin"'])
        current.write("testing service stop...")
        admin.run(current, args=['stop', 'TestService'])
        current.writeln("ok")
        current.write("testing service start...")
        admin.run(current, args=['start', 'TestService'])
        current.writeln("ok")
        current.write("testing shutdown...")
        admin.run(current, args=['shutdown'])
        current.writeln("ok")

TestSuite(__name__, [
        ClientServerTestCase(server=IceBox(args=['--Ice.Config="{testdir}/config.icebox"'])),
        IceBoxAdminTestCase("iceboxadmin", server=IceBox(args=['--Ice.Config="{testdir}/config.icebox"'])),
    ],
    libDirs=["testservice"],
    runOnMainThread=True,
    options={ "ipv6" : [False], "mx" : [False], "cpp11" : [False] },
    multihost=False)
