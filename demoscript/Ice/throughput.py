#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys
from demoscript import Util

def runseries(client):
    client.expect('==> ', timeout=240)
    client.sendline('t')

    client.expect('==> ', timeout=2000)
    print("twoway: %s " % (client.before))
    if not Util.fast:
        client.sendline('o')

        client.expect('==> ', timeout=2000)
        print("oneway: %s " % (client.before))

        client.sendline('r')

        client.expect('==> ', timeout=2000)
        print("receive: %s" % (client.before))
        client.sendline('e')

        client.expect('==> ', timeout=2000)
        print("echo: %s" % (client.before))

def run(client, server):
    print("testing bytes")
    runseries(client)

    print("testing strings")
    client.sendline('2')
    runseries(client)

    print("testing structs with string")
    client.sendline('3')
    runseries(client)

    print("testing structs with two ints and double")
    client.sendline('4')
    runseries(client)

    client.sendline('s')
    server.waitTestSuccess()

    client.sendline('x')
    client.waitTestSuccess()
