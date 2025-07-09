# Copyright (c) ZeroC, Inc.

import os

from IceGridUtil import IceGridTestCase
from Util import TestSuite, Windows, platform

if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(
        __file__,
        [IceGridTestCase(exevars={"verifier.dir": "verifier"})],
        runOnMainThread=True,
        multihost=False,
    )
