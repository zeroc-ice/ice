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

def runseries(client):
    print("testing bytes...")
    client.expect('==> ', timeout=240)
    client.sendline('e')
    client.expect('==> ', timeout=2000)
    print("echo: %s " % (client.before))

    print("testing strings...")
    client.sendline('2')
    client.expect('==> ', timeout=240)
    client.sendline('e')
    client.expect('==> ', timeout=2000)
    print("echo: %s " % (client.before))

    print("testing structs with string...")
    client.sendline('3')
    client.expect('==> ', timeout=240)
    client.sendline('e')
    client.expect('==> ', timeout=2000)
    print("echo: %s " % (client.before))

    print("testing structs with two ints and double...")
    client.sendline('4')
    client.expect('==> ', timeout=240)
    client.sendline('e')
    client.expect('==> ', timeout=2000)
    print("echo: %s " % (client.before))

def run(client, server):
    print("testing with 2 outstanding requests\n")
    runseries(client)

    print("testing with unlimited outstanding requests\n")
    client.sendline('o')
    runseries(client)

    client.sendline('s')
    server.waitTestSuccess()

    client.sendline('x')
    client.waitTestSuccess()
