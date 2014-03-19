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

def run(clientStr, server):
    client = Util.spawn(clientStr)
    client.expect('==>')
    client.sendline('foo')

    sys.stdout.write("testing session creation... ")
    sys.stdout.flush()
    server.expect('The session foo is now created.')
    client.sendline('c')
    client.sendline('0')
    server.expect("Hello object #0 for session `foo' says:\nHello foo!")
    client.sendline('1')
    client.expect('Index is too high')
    client.sendline('x')
    client.waitTestSuccess()
    server.expect("The session foo is now destroyed.")
    print("ok")

    sys.stdout.write("testing session cleanup... ")
    sys.stdout.flush()
    client = Util.spawn(clientStr)
    client.expect('==>')
    client.sendline('foo')
    server.expect('The session foo is now created.')
    client.sendline('c')
    client.sendline('t')
    client.waitTestSuccess()
    server.expect("The session foo is now destroyed.\n.*The session foo has timed out.", timeout=25)
    print("ok")

    client = Util.spawn(clientStr)
    client.expect('==>')
    client.sendline('foo')
    server.expect('The session foo is now created.')
    client.sendline('s')
    server.waitTestSuccess()

    client.waitTestSuccess()
