#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, demoscript, signal
import demoscript.pexpect as pexpect

def runDemo(clientCmd, serverCmd):
    server = demoscript.Util.spawn(serverCmd + ' --Ice.PrintAdapterReady')
    server.expect('Discover ready')
    server.expect('Hello ready')

    client = demoscript.Util.spawn(clientCmd)
    server.expect('Hello World!')
    client.waitTestSuccess()

    client = demoscript.Util.spawn(clientCmd)
    server.expect('Hello World!')
    client.waitTestSuccess()

    client = demoscript.Util.spawn(clientCmd)
    server.expect('Hello World!')
    client.waitTestSuccess()

    server.kill(signal.SIGINT)
    server.waitTestSuccess()

def run(clientCmd, serverCmd):
    print "testing multicast discovery (Ipv4)...",
    sys.stdout.flush()
    runDemo(clientCmd, serverCmd)
    print "ok"

    print "testing multicast discovery (IPv6)...",
    sys.stdout.flush()
    serverCmd += " --Ice.IPv6=1 --Discover.Endpoints='udp -h \"ff01::1:1\" -p 10000'"
    clientCmd += " --Ice.IPv6=1 --Discover.Proxy='discover:udp -h \"ff01::1:1\" -p 10000'"
    runDemo(clientCmd, serverCmd)
    print "ok"
