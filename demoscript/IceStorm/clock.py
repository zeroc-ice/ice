#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, time, signal, demoscript
import demoscript.pexpect as pexpect

def runtest(icestorm, subCmd, subargs, pubCmd, pubargs):
    print "testing pub%s/sub%s..." % (pubargs, subargs),
    sys.stdout.flush()
    sub = demoscript.Util.spawn('%s --Ice.PrintAdapterReady %s' %(subCmd, subargs))
    sub.expect('.* ready')

    icestorm.expect('subscribeAndGetPublisher:')

    pub = demoscript.Util.spawn('%s %s' %(pubCmd, pubargs))

    pub.expect('publishing tick events')
    time.sleep(3)
    sub.expect('[0-9][0-9]/[0-9][0-9].*\r{1,2}\n[0-9][0-9]/[0-9][0-9]')

    pub.kill(signal.SIGINT)
    pub.waitTestSuccess()

    sub.kill(signal.SIGINT)
    sub.waitTestSuccess()
    try:
        icestorm.expect('unsubscribe:')
    except pexpect.TIMEOUT:
        print "(Wait for Unsubscribe failed, expected for Mono)",
    print "ok"

def run(subCmd, pubCmd):
    print "cleaning databases...",
    sys.stdout.flush()
    demoscript.Util.cleanDbDir("db")
    print "ok"

    if demoscript.Util.defaultHost:
        args = ' --IceBox.UseSharedCommunicator.IceStorm=1'
    else:
        args = ''

    icestorm = demoscript.Util.spawn('%s --Ice.Config=config.icebox --Ice.PrintAdapterReady %s' % (demoscript.Util.getIceBox(), args), language="C++")
    icestorm.expect('.* ready')

    runtest(icestorm, subCmd, "", pubCmd, "")

    subargs = [" --oneway", " --twoway", " --datagram", " --twoway", " --ordered", " --batch"]
    for s in subargs:
        runtest(icestorm, subCmd, s, pubCmd, "")
    pubargs = [" --oneway", " --datagram", " --twoway" ]
    for s in pubargs:
        runtest(icestorm, subCmd, "", pubCmd, s)

    admin = demoscript.Util.spawn('iceboxadmin --Ice.Config=config.icebox shutdown', language="C++")
    admin.waitTestSuccess()
    icestorm.waitTestSuccess()
