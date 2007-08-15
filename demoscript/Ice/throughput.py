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

def runseries(client):
    client.expect('==> ', timeout=240)
    client.sendline('t')
    client.expect('t')

    client.expect('==> ', timeout=240)
    print "twoway: %s " % (client.before)

    client.sendline('o')

    client.expect('o')
    client.expect('==> ', timeout=240)
    print "oneway: %s " % (client.before)

    client.sendline('r')
    client.expect('r')

    client.expect('==> ', timeout=240)
    print "receive: %s" % (client.before)
    client.sendline('e')
    client.expect('e')

    client.expect('==> ', timeout=240)
    print "echo: %s" % (client.before)

def run(client, server):
    print "testing bytes"
    runseries(client)

    if not demoscript.Util.fast:
        print "testing strings"
        client.sendline('2')
        runseries(client)

        print "testing structs with string... "
        client.sendline('3')
        runseries(client)

        print "testing structs with two ints and double... "
        client.sendline('4')
        runseries(client)

    client.sendline('s')
    server.expect(pexpect.EOF)
    assert server.wait() == 0

    client.sendline('x')
    client.expect(pexpect.EOF)
    assert client.wait() == 0
