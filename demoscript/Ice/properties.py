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
from demoscript import *
from scripts import Expect

def runtests(client, server):
    sys.stdout.write("testing setProperties...")
    sys.stdout.flush()
    client.sendline('1')
    client.expect('Demo.Prop1 is now 1.*Demo.Prop2 is now 2.*Demo.Prop3 is now 3')
    client.sendline('1')
    client.expect('None')
    print("ok")
    sys.stdout.write("testing resetProperties...")
    sys.stdout.flush()
    client.sendline('2')
    client.expect('Demo.Prop1 is now 10.*Demo.Prop2 was removed.*Demo.Prop3 is now 30')
    client.sendline('2')
    client.expect('None')
    print("ok")

def run(client, server):
    runtests(client, server)

    client.sendline('s')
    server.waitTestSuccess()

    client.sendline('x')
    client.waitTestSuccess()
