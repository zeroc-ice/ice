#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, time, signal
from scripts import Expect

def run(client, server, glacier2):
    print "testing ",
    sys.stdout.flush()
    client.expect('user id:')
    client.sendline("foo")
    client.expect('password:')
    client.sendline("foo")

    client.expect("==>")

    print "twoway",
    sys.stdout.flush()
    client.sendline('t')
    server.expect('initiating callback to')
    client.expect('received callback')
    #glacier2.expect('_fwd/t \\]')

    print "oneway",
    sys.stdout.flush()
    client.sendline('o')
    server.expect('initiating callback to')
    client.expect('received callback')
    #glacier2.expect('_fwd/o \\]')

    print "batch",
    sys.stdout.flush()
    client.sendline('O')
    try:
        server.expect('initiating callback to', timeout=1)
    except Expect.TIMEOUT:
        pass
    client.sendline('O')
    client.sendline('f')
    #glacier2.expect('_fwd/O \\]')
    print "ok"

    print "testing override context field...",
    sys.stdout.flush()
    client.sendline('v')
    client.sendline('t')
    #glacier2.expect('_fwd/t, _ovrd/some_value')
    server.expect('initiating callback to')
    client.expect('received callback')
    print "ok"

    print "testing fake category...",
    sys.stdout.flush()
    client.sendline('v')
    client.sendline('F')
    client.sendline('t')
    server.expect('initiating callback to.*fake.*ObjectNotExistException')
    try:
        client.expect('received callback', timeout=1)
    except Expect.TIMEOUT:
        pass
    print "ok"


    # SessionNotExist
    client.sendline('x')
    client.waitTestSuccess()

    server.kill(signal.SIGINT)
    server.waitTestSuccess()

    glacier2.kill(signal.SIGINT)
    glacier2.waitTestSuccess()
