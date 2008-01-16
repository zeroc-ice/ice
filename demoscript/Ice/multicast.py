#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, demoscript, signal
import demoscript.pexpect as pexpect

def run(clientCmd, serverCmd):
    print "testing multicast discovery...",
    sys.stdout.flush()

    server = demoscript.Util.spawn(serverCmd + ' --Ice.PrintAdapterReady')
    server.expect('.* ready')
    server.expect('.* ready')

    client = demoscript.Util.spawn(clientCmd)
    server.expect('Hello World!')
    client.waitTestSuccess()

    client = demoscript.Util.spawn(clientCmd)
    server.expect('Hello World!')
    client.waitTestSuccess()

    client = demoscript.Util.spawn(clientCmd)
    server.expect('Hello World!')
    client.waitTestSuccess()

    server.kill(signal.SIGQUIT)
    
    print "ok"
