# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from IceBoxUtil import IceBox
from Util import ClientServerTestCase, TestSuite


TestSuite(
    __name__,
    [
        ClientServerTestCase(
            "client/server #1", server=IceBox("{testdir}/config.icebox")
        ),
        ClientServerTestCase(
            "client/server #2", server=IceBox("{testdir}/config.icebox2")
        ),
    ],
    libDirs=["testservice"],
    runOnMainThread=True,
    options={"protocol": ["tcp"], "ipv6": [False], "mx": [False]},
    multihost=False,
)
