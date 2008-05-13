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

def runClient(clientCmd, server1, server2):
    client = demoscript.Util.spawn(clientCmd)
    received = False
    try:
        server1.expect('Hello World!')
        received = True
    except pexpect.TIMEOUT:
        pass
    try:
        server2.expect('Hello World!')
        received = True
    except pexpect.TIMEOUT:
        pass
    if not received:
        raise pexpect.TIMEOUT
    client.waitTestSuccess()

def runDemo(clientCmd, serverCmd):
    server1 = demoscript.Util.spawn(serverCmd + ' --Ice.PrintAdapterReady')
    server1.expect('Discover ready')
    server1.expect('Hello ready')

    server2 = demoscript.Util.spawn(serverCmd + ' --Ice.PrintAdapterReady')
    server2.expect('Discover ready')
    server2.expect('Hello ready')

    runClient(clientCmd, server1, server2)
    runClient(clientCmd, server1, server2)
    runClient(clientCmd, server1, server2)

    server1.kill(signal.SIGINT)
    server1.waitTestSuccess()

    server2.kill(signal.SIGINT)
    server2.waitTestSuccess()

def run(clientCmd, serverCmd):
    print "testing multicast discovery (Ipv4)...",
    sys.stdout.flush()
    runDemo(clientCmd, serverCmd)
    print "ok"

    if demoscript.Util.defaultLanguage == "Java" and demoscript.Util.isCygwin():
        print "skipping testing multicast discovery (IPv6) under windows...",
    else:
        print "testing multicast discovery (IPv6)...",
        sys.stdout.flush()
        serverCmd += " --Ice.IPv6=1 --Discover.Endpoints='udp -h \"ff01::1:1\" -p 10000'"
        clientCmd += " --Ice.IPv6=1 --Discover.Proxy='discover:udp -h \"ff01::1:1\" -p 10000'"
        runDemo(clientCmd, serverCmd)
    print "ok"
