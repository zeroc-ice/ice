# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************


TestSuite(__name__, [
    ClientTestCase("client with default encoding"),
    ClientTestCase("client with 1.0 encoding", props={ "Ice.Default.EncodingVersion" : "1.0" })
])