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

def run(clientStr, serverStr):
    print "cleaning databases...",
    sys.stdout.flush()
    demoscript.Util.cleanDbDir("db")
    print "ok"

    print "starting server...",
    sys.stdout.flush()
    server = demoscript.Util.spawn(serverStr + ' --Ice.PrintAdapterReady --Freeze.Warn.Deadlocks=0')
    server.expect('Casino ready')
    print "ok"

    print "starting client1...",
    sys.stdout.flush()
    client1 = demoscript.Util.spawn(clientStr)
    client1.expect('Retrieve bank and players... ok')
    print "ok"

    print "starting client2...",
    sys.stdout.flush()
    client2 = demoscript.Util.spawn(clientStr)
    client2.expect('Retrieve bank and players... ok')
    print "ok"

    print "gambling...",
    sys.stdout.flush()
    client1.expect('All chips accounted for\? yes\r{1,2}\nEach player buys 3,000 chips... ok\r{1,2}\nAll chips accounted for\? yes');
    client2.expect('All chips accounted for\? yes\r{1,2}\nEach player buys 3,000 chips... ok\r{1,2}\nAll chips accounted for\? yes');

    client1.expect('All chips accounted for\? yes\r{1,2}\nSleep for 2 seconds', timeout=200)
    client2.expect('All chips accounted for\? yes\r{1,2}\nSleep for 2 seconds', timeout=200)

    client1.expect('All chips accounted for\? yes', timeout=200)
    client2.expect('All chips accounted for\? yes', timeout=200)
    print "ok"

    print "shutting down...",
    sys.stdout.flush()
    client1.waitTestSuccess()
    client2.waitTestSuccess()
    server.kill(signal.SIGINT)
    server.waitTestSuccess()
    print "ok"
