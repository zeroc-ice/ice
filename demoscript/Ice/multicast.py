#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys
import demoscript.pexpect as pexpect

def run(client, server1, server2, server3):
    print "testing multicast...",
    sys.stdout.flush()

    client.sendline('t')
    server1.expect('Hello World!')
    server2.expect('Hello World!')
    server3.expect('Hello World!')

    client.sendline('t')
    server1.expect('Hello World!')
    server2.expect('Hello World!')
    server3.expect('Hello World!')

    client.sendline('s')
    server1.waitTestSuccess()
    server2.waitTestSuccess()
    server3.waitTestSuccess()

    client.sendline('x')
    client.waitTestSuccess()

    print "ok"
