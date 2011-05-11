#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys
from demoscript import *
from scripts import Expect

def run(client, server):
    print "testing...",
    sys.stdout.flush()

    client.sendline('t')
    server.expect('initiating callback')
    client.expect('received callback')

    client.sendline('t')
    server.expect('initiating callback')
    client.expect('received callback')

    print "ok"

    client.sendline('s')
    server.waitTestSuccess()

    client.sendline('x')
    client.waitTestSuccess()
