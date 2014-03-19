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

def run(client, server):
    sys.stdout.write("testing client... ")
    sys.stdout.flush()
    client.sendline('1')
    client.expect('==> ', timeout=2000)
    client.sendline('2')
    client.expect('==> ', timeout=2000)
    client.sendline('3')
    client.expect('==> ', timeout=2000)
    client.sendline('s')
    server.waitTestSuccess()

    client.expect('==> ', timeout=2000)
    client.sendline('x')
    client.waitTestSuccess()
    print("ok")
