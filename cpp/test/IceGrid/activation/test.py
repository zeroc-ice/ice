# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(__file__, [IceGridTestCase()], multihost=False)
