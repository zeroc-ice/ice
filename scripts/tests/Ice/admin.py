# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

# TODO: remove the transport option and update the test so that the server can use the client.p12
# cert when calling the client's OA
TestSuite(__name__, options = { "transport": ["tcp", "ws"] }, multihost=False)
