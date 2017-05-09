# -*- coding: utf-8 -*-
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

def setlimits():
    if not isinstance(platform, Windows):
        #
        # Set a low limit for the number of available file descriptors. This is needed for the max FD test
        # which could otherwise potentially allocate many file descriptors on the system.
        #
        import resource
        resource.setrlimit(resource.RLIMIT_NOFILE, (92, 92))

TestSuite(__name__, [ClientServerTestCase(server=Server(preexec_fn=setlimits))])
