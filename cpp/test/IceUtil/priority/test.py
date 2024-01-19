#
# Copyright (c) ZeroC, Inc. All rights reserved.
#


import os
from Util import TestSuite, platform, Windows, Darwin


if not isinstance(platform, Darwin) and (
    isinstance(platform, Windows) or os.getuid() == 0
):
    TestSuite(__name__)
