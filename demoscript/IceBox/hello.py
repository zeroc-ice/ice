#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import pexpect, sys, demoscript

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
    except pexpect.TIMEOUT:
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
        except pexpect.TIMEOUT:
            pass
        client.sendline('D')
        client.sendline('f')
        server.expect('Hello World!')
        server.expect('Hello World!')
    print "... ok"

def run(client, server):
    runtests(client, server, False)

    print "repeating tests with SSL"

    client.sendline('S')

    runtests(client, server, True)

    client.sendline('x')
    client.expect(pexpect.EOF)
    assert client.wait() == 0

    admin = demoscript.Util.spawn('iceboxadmin --IceBox.InstanceName=DemoIceBox --IceBox.ServiceManager.Endpoints="tcp -p 9998:ssl -p 9999" shutdown')

    admin.expect(pexpect.EOF)
    assert admin.wait() == 0

    server.expect(pexpect.EOF)
    assert server.wait() == 0
