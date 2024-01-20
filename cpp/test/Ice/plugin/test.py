#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from Util import ClientTestCase, SimpleClient, TestSuite


TestSuite(
    __file__,
    [
        ClientTestCase(
            client=SimpleClient(
                args=lambda process, current: [current.getBuildDir("testplugin")]
            )
        )
    ],
    libDirs=["testplugin"],
)
