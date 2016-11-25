# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

TestSuite(__name__, [
    ClientServerTestCase("client/server with compact format"),
    ClientServerTestCase("client/server with sliced format", props={ "Ice.Default.SlicedFormat" : True }),
    ClientServerTestCase("client/server with 1.0 encoding", props={ "Ice.Default.EncodingVersion" : "1.0" }),
])