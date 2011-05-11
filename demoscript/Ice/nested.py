#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, signal
from demoscript import *
from scripts import Expect

def run(client, server):
    print "testing nested...",
    sys.stdout.flush()
    client.sendline('1')
    server.expect('1')
    client.expect('.*for exit:')
    client.sendline('2')
    server.expect('2')
    client.expect('1\n.*for exit:')
    client.sendline('3')
    server.expect('3\n1')
    client.expect('2\n.*for exit:')
    print "ok"

    print "testing blocking...",
    sys.stdout.flush()
    client.sendline('21') # This will cause a block.
    server.expect('\n'.join(['13', '11', '9', '7', '5', '3']))
    client.expect('\n'.join(['12', '10', '8', '6', '4', '2']))
    client.expect('TimeoutException', timeout=3000)
    server.expect('TimeoutException', timeout=3000)
    print "ok"

    client.sendline('x')
    client.waitTestSuccess()

    server.kill(signal.SIGINT)
    server.waitTestSuccess()
