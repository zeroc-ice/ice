#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, signal
from demoscript import Util
import Expect

def run(clientStr, server):
    sys.stdout.write("adding client 1... ")
    sys.stdout.flush()
    client1 = Util.spawn(clientStr)
    server.expect('adding client')
    client1.expect('received callback #1')
    print("ok")

    sys.stdout.write("adding client 2... ")
    sys.stdout.flush()
    client2 = Util.spawn(clientStr)
    server.expect('adding client')
    client1.expect('received callback #')
    client2.expect('received callback #')
    print("ok")

    sys.stdout.write("removing client 2... ")
    sys.stdout.flush()
    client2.kill(signal.SIGINT)
    client2.waitTestSuccess(timeout=20)
    server.expect('removing client')
    client1.expect('received callback #')
    print("ok")

    sys.stdout.write("removing client 1... ")
    sys.stdout.flush()
    client1.kill(signal.SIGINT)
    client1.waitTestSuccess()
    server.expect('removing client')

    server.kill(signal.SIGINT)
    server.waitTestSuccess(timeout=30)

    print("ok")
