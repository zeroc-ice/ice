#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, signal
from demoscript import Util, Expect

def runClient(clientCmd, server1, server2):
    client = Util.spawn(clientCmd)
    received = False
    ex = None
    for i in range(0, 20):
        try:
            server1.expect('Hello World!', 1)
            received = True
        except Expect.TIMEOUT:
            pass
        try:
            server2.expect('Hello World!', 1)
            received = True
        except Expect.TIMEOUT as e:
            ex = e
            pass

        if received:
            break

    if not received:
        raise ex
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
    sys.stdout.write("testing multicast discovery (IPv4)... ")
    sys.stdout.flush()
    if serverCmd.startswith("java"):
        runDemo(clientCmd, "java -Djava.net.preferIPv4Stack=true -jar build/libs/server.jar")
    else:
        runDemo(clientCmd, serverCmd)
    print("ok")

    #
    # No IPv6 support in Windows with Java 1.6.x
    #
    if Util.getMapping() != "java" or not Util.isWin32() or not Util.getJavaVersion().startswith("1.6"):

        sys.stdout.write("testing multicast discovery (IPv6)... ")
        sys.stdout.flush()

        endpoint = 'udp -h \\"ff15::1:1\\" -p 10000'
        #
        # On OS X, if the host doesn't have global scoped IPv6
        # address, the system can't figure out the default interface
        # to use when joining the multicast group. We specify it
        # explicitly here to the loopback, this makes sure the demos
        # work also on hosts on hosts without a global scope address.
        #
        if Util.isDarwin():
            endpoint += ' --interface \\"::1\\"'

        serverCmd += ' --Ice.IPv6=1 --Discover.Endpoints="%s"' % (endpoint)
        clientCmd += ' --Ice.IPv6=1 --Discover.Proxy="discover:%s"' % (endpoint)

        runDemo(clientCmd, serverCmd)
        print("ok")
