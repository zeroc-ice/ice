#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, time
import Expect

def runtests(client, server):
    sys.stdout.write("testing interrupts... ")
    sys.stdout.flush()
    client.sendline('b')
    server.expect('starting task 0')
    time.sleep(1)
    client.sendline('i')
    client.expect('blocking task 0 interrupted')
    client.sendline('t')
    server.expect('starting task 1')
    time.sleep(1)
    client.sendline('s')
    client.expect('task 1 completed running')
    server.expect('interrupted task 0')
    print("ok")

def run(client, server):
    runtests(client, server)

    server.waitTestSuccess()
    client.sendline('x')
    client.waitTestSuccess()
