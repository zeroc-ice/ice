#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys
from demoscript import *
from scripts import Expect

def runtests(client, server, secure):
    print "testing twoway",
    sys.stdout.flush()
    client.sendline('t')
    server.expect('Hello World!')
    print "oneway",
    sys.stdout.flush()
    client.sendline('o')
    server.expect('Hello World!')
    if not secure:
        print "datagram",
        sys.stdout.flush()
        client.sendline('d')
        server.expect('Hello World!')
    print "... ok"

    print "testing batch oneway",
    sys.stdout.flush()
    client.sendline('O')
    try:
        server.expect('Hello World!', timeout=1)
    except Expect.TIMEOUT:
        pass
    client.sendline('O')
    client.sendline('f')
    server.expect('Hello World!')
    server.expect('Hello World!')
    if not secure:
        print "datagram",
        sys.stdout.flush()
        client.sendline('D')
        try:
            server.expect('Hello World!', timeout=1)
        except Expect.TIMEOUT:
            pass
        client.sendline('D')
        client.sendline('f')
        server.expect('Hello World!')
        server.expect('Hello World!')
    print "... ok"

    print "testing timeout...", 
    sys.stdout.flush()
    client.sendline('T')
    client.sendline('P')
    client.expect('server delay is now set to 2500ms')
    client.sendline('t')
    client.expect('.*TimeoutException.*', timeout=10)
    server.expect('Hello World!')
    server.expect('Hello World!') # second because op is idempotent
    client.sendline('P')
    client.expect('server delay is now deactivated')
    client.sendline('t')
    server.expect('Hello World!')
    client.sendline('T')
    print "ok"

def run(client, server):
    runtests(client, server, False)

    print "repeating tests with SSL"

    client.sendline('S')

    runtests(client, server, True)

    client.sendline('s')
    server.waitTestSuccess()

    client.sendline('x')
    client.waitTestSuccess()
