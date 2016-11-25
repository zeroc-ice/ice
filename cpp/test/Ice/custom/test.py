# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

# Don't run this test with SSL, it doesn't work on Windows, see ICE-4588
TestSuite(__name__, options = { "protocol" : ["tcp", "ws"] })
