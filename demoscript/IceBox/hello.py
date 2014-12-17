#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, demoscript
import Expect

def runtests(client, server, secure):
    sys.stdout.write("testing twoway ")
    sys.stdout.flush()
    client.sendline('t')
    server.expect('Hello World!')
    sys.stdout.write("oneway ")
    sys.stdout.flush()
    client.sendline('o')
    server.expect('Hello World!')
    if not secure:
        sys.stdout.write("datagram ")
        sys.stdout.flush()
        client.sendline('d')
        server.expect('Hello World!')
    print("... ok")

    sys.stdout.write("testing batch oneway ")
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
        sys.stdout.write("datagram ")
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
    print("... ok")

def run(client, server):
    runtests(client, server, False)

    if not demoscript.Util.isMono():
        print("repeating tests with SSL")

        client.sendline('S')

        runtests(client, server, True)

    client.sendline('x')
    client.waitTestSuccess()

    admin = demoscript.Util.spawn('iceboxadmin --IceBoxAdmin.ServiceManager.Proxy="DemoIceBox/admin -f IceBox.ServiceManager:tcp -p 9996 -h 127.0.0.1" shutdown')

    admin.waitTestSuccess()
    server.waitTestSuccess()
