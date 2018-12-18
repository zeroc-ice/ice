# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

if not isinstance(platform, Darwin) and (isinstance(platform, Windows) or os.getuid() == 0):
    TestSuite(__name__)
