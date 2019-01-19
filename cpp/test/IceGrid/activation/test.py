# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(__file__, [IceGridTestCase()], multihost=False)
