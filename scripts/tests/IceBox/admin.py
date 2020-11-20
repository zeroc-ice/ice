# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

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

# TODO: add support for running the test with ice2
TestSuite(__name__, [
        ClientServerTestCase(server=IceBox("{testdir}/config.icebox")),
        IceBoxAdminTestCase("iceboxadmin", server=IceBox("{testdir}/config.icebox")),
    ],
    libDirs=["testservice"],
    runOnMainThread=True,
    options={ "ipv6" : [False], "mx" : [False], "protocol" : ["ice1"], "transport" : ["tcp", "ws"] },
    multihost=False)
