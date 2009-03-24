#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
    server.expect('Hello World from id: 1')
    server.expect('name: "Fred Jones"')
    server.expect('email: "fred@jones.com"')
    print "ok"

    print "shutting down...",
    client.sendline('s')
    server.waitTestSuccess()

    client.sendline('x')
    client.waitTestSuccess()
    print "ok"
