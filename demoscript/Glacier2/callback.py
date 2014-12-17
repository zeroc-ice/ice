#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, time, signal
from scripts import Expect

def run(client, server, glacier2):
    sys.stdout.write("testing ")
    sys.stdout.flush()
    client.expect('user id:')
    client.sendline("foo")
    client.expect('password:')
    client.sendline("foo")

    client.expect("==>")

    sys.stdout.write("twoway ")
    sys.stdout.flush()
    client.sendline('t')
    server.expect('initiating callback to')
    client.expect('received callback')
    glacier2.expect('_fwd/t')

    sys.stdout.write("oneway ")
    sys.stdout.flush()
    client.sendline('o')
    server.expect('initiating callback to')
    client.expect('received callback')
    glacier2.expect('_fwd/o')

    sys.stdout.write("batch ")
    sys.stdout.flush()
    client.sendline('O')
    client.sendline('O')
    client.sendline('f')
    try:
        server.expect('initiating callback to', timeout=1)
    except Expect.TIMEOUT:
        pass
    glacier2.expect('_fwd/O')
    print("ok")

    sys.stdout.write("testing override context field... ")
    sys.stdout.flush()
    client.sendline('v')
    client.sendline('t')
    glacier2.expect('_fwd/t, _ovrd/some_value')
    server.expect('initiating callback to')
    client.expect('received callback')
    print("ok")

    sys.stdout.write("testing fake category... ")
    sys.stdout.flush()
    client.sendline('v')
    client.sendline('F')
    client.sendline('t')
    server.expect('initiating callback to.*fake.*ObjectNotExistException')
    try:
        client.expect('received callback', timeout=1)
    except Expect.TIMEOUT:
        pass
    print("ok")

    client.sendline('s')
    server.expect('shutting down...')
    server.waitTestSuccess()

    client.sendline('x')
    glacier2.expect('destroying session')
    client.waitTestSuccess()

    glacier2.kill(signal.SIGINT)
    glacier2.waitTestSuccess()
