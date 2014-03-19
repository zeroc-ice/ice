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

def runtests(client, server, secure):
    sys.stdout.write("testing greeting... ")
    sys.stdout.flush()
    client.sendline('g')
    server.expect('Hello there!')
    client.sendline('g')
    server.expect('Hello there!')
    print("ok")

    sys.stdout.write("testing null greeting... ")
    sys.stdout.flush()
    client.sendline('t')
    client.sendline('g')
    server.expect('Received null greeting')
    client.sendline('g')
    server.expect('Received null greeting')
    print("ok")

def run(client, server):
    runtests(client, server, False)

    client.sendline('s')
    server.waitTestSuccess()

    client.sendline('x')
    client.waitTestSuccess()
