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
import Expect

def runDemo(client, server):
    sys.stdout.write("testing request contexts ")
    sys.stdout.flush()
    client.sendline('1')
    server.expect('None')
    client.sendline('2')
    server.expect('Explicit')
    client.sendline('3')
    server.expect('Per-Proxy')
    client.sendline('4')
    server.expect('Implicit')
    print("... ok")

def run(client, server):
    runDemo(client, server)

    client.sendline('s')
    server.waitTestSuccess()
    print("server shutdown");
    client.sendline('x')
    client.waitTestSuccess()
    print("client shutdown");
