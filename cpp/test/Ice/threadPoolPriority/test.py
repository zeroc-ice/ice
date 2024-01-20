#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import os
from Util import ClientServerTestCase, Darwin, TestSuite, Windows, platform


if not isinstance(platform, Darwin) and (
    isinstance(platform, Windows) or os.getuid() == 0
):
    TestSuite(
        __file__,
        [
            ClientServerTestCase(),
            ClientServerTestCase(name="client/custom server", server="servercustom"),
        ],
        options={"mx": [False]},
    )
