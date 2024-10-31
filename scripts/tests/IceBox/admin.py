# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from IceBoxUtil import IceBox, IceBoxAdmin
from Util import ClientServerTestCase, TestSuite


class IceBoxAdminTestCase(ClientServerTestCase):
    def runClientSide(self, current):
        admin = IceBoxAdmin(args=['--Ice.Config="{testdir}/config.admin"'])
        current.write("testing service stop...")
        admin.run(current, args=["stop", "TestService"])
        current.writeln("ok")
        current.write("testing service start...")
        admin.run(current, args=["start", "TestService"])
        current.writeln("ok")
        current.write("testing shutdown...")
        admin.run(current, args=["shutdown"])
        current.writeln("ok")


TestSuite(
    __name__,
    [
        # readyCount=2 for Ice.Admin and the "bundleName" from IceBox.PrintServicesReady
        ClientServerTestCase(server=IceBox("{testdir}/config.icebox", readyCount=2)),
        IceBoxAdminTestCase("iceboxadmin", server=IceBox("{testdir}/config.icebox", readyCount=2)),
    ],
    libDirs=["testservice"],
    runOnMainThread=True,
    options={"ipv6": [False], "mx": [False]},
    multihost=False,
)
