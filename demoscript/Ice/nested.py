#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import pexpect, sys, signal

def run(client, server):
    print "testing nested...",
    sys.stdout.flush()
    client.sendline('1')
    server.expect('1')
    client.expect('.*for exit:')
    client.sendline('2')
    server.expect('2')
    client.expect('1\r{1,2}\n.*for exit:')
    client.sendline('3')
    server.expect('3\r{1,2}\n1')
    client.expect('2\r{1,2}\n.*for exit:')
    print "ok"

    print "testing blocking...",
    sys.stdout.flush()
    client.sendline('21') # This will cause a block.
    server.expect('\r{1,2}\n'.join(['13', '11', '9', '7', '5', '3']))
    client.expect('\r{1,2}\n'.join(['12', '10', '8', '6', '4', '2']))
    client.expect('TimeoutException', timeout=3000)
    server.expect('TimeoutException', timeout=3000)
    print "ok"

    client.sendline('x')
    client.expect(pexpect.EOF)
    assert client.wait() == 0

    server.kill(signal.SIGINT)
    server.expect(pexpect.EOF)
    status = server.wait()
    assert status == 0 or status == 130 or server.signalstatus == signal.SIGINT
