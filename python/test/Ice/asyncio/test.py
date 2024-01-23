# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#


# This test requires asyncio methods which are only available with Python 3.7

from Util import TestSuite, currentConfig


if currentConfig.getPythonVersion() >= (3, 7):
    TestSuite(__name__)
