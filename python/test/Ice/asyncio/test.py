# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#


# This test requires asyncio methods which are only available with Python 3.7

if currentConfig.getPythonVersion() >= (3, 7):
    TestSuite(__name__)
