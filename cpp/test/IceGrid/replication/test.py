# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

clientProps = lambda process, current: { "ServerDir" : current.getBuildDir("server") }

if isinstance(platform, Windows) or os.getuid() != 0:
    TestSuite(__file__, [ IceGridTestCase(client=IceGridClient(props=clientProps)) ], runOnMainThread=True, multihost=False)
