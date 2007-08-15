#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import pexpect, sys

def run(client, server):
    print "testing client... ",
    sys.stdout.flush()
    client.sendline('i')
    server.expect('Hello World!')
    client.sendline('d')
    try:
        server.expect('Hello World!', timeout=1)
    except pexpect.TIMEOUT:
        pass
    client.sendline('i')
    server.expect('Hello World!')
    server.expect('Hello World!')
    print "ok"

    print "testing shutdown... ",
    sys.stdout.flush()
    client.sendline('d')
    client.sendline('s')
    server.expect(pexpect.EOF)
    assert server.wait() == 0

    client.expect('RequestCanceledException')
    client.sendline('x')
    client.expect(pexpect.EOF)
    assert client.wait() == 0
    print "ok"
