#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, signal
from demoscript import *
from scripts import Expect

def runClient(clientCmd, server1, server2):
    client = Util.spawn(clientCmd)
    received = False
    try:
        server1.expect('Hello World!')
        received = True
    except Expect.TIMEOUT:
        pass
    try:
        server2.expect('Hello World!')
        received = True
    except Expect.TIMEOUT:
        pass
    if not received:
        raise Expect.TIMEOUT
    client.waitTestSuccess()

def runDemo(clientCmd, serverCmd):
    server1 = Util.spawn(serverCmd + ' --Ice.PrintAdapterReady')
    server1.expect('Discover ready')
    server1.expect('Hello ready')

    server2 = Util.spawn(serverCmd + ' --Ice.PrintAdapterReady')
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
    if serverCmd.startswith("java"):
        runDemo(clientCmd, "java -Djava.net.preferIPv4Stack=true Server")
    else:
        runDemo(clientCmd, serverCmd)
    print "ok"

    if Util.getMapping() == "java" and Util.isWin32():
        print "skipping testing multicast discovery (IPv6) under windows...",
    else:
        print "testing multicast discovery (IPv6)...",
        sys.stdout.flush()
        serverCmd += ' --Ice.IPv6=1 --Discover.Endpoints="udp -h \\"ff01::1:1\\" -p 10000"'
        clientCmd += ' --Ice.IPv6=1 --Discover.Proxy="discover:udp -h \\"ff01::1:1\\" -p 10000"'
        runDemo(clientCmd, serverCmd)
    print "ok"
