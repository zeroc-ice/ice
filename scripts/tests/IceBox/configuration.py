# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

# TODO: support for different protocols? For services without inheritance, we have to somehow set the
# default protocol in the service configuration
TestSuite(__name__, [
    ClientServerTestCase("client/server #1", server=IceBox("{testdir}/config.icebox")),
    ClientServerTestCase("client/server #2", server=IceBox("{testdir}/config.icebox2"))
],
libDirs=["testservice"],
runOnMainThread=True,
options={ "transport" : ["tcp"], "ipv6" : [False], "mx" : [False], "protocol": ["ice2"] },
multihost=False)
