# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

if not isinstance(platform, Darwin) and (isinstance(platform, Windows) or os.getuid() == 0):
    TestSuite(__file__, [
        ClientServerTestCase(),
        ClientServerTestCase(name="client/custom server", server="servercustom")
    ], options={ "mx" : [False] })
