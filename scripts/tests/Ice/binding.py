# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

def setlimits():
    if not isinstance(platform, Windows):
        #
        # Set a low limit for the number of available file descriptors. This is needed for the max FD test
        # which could otherwise potentially allocate many file descriptors on the system.
        #
        import resource
        resource.setrlimit(resource.RLIMIT_NOFILE, (92, 92))

TestSuite(__name__, [ClientServerTestCase(server=Server(preexec_fn=setlimits))])
