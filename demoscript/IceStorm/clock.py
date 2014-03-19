#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, time, signal
from demoscript import Util

def runtest(icestorm, subCmd, subargs, pubCmd, pubargs):
    sys.stdout.write("testing pub%s/sub%s... " % (pubargs, subargs))
    sys.stdout.flush()
    sub = Util.spawn('%s --Ice.PrintAdapterReady %s' %(subCmd, subargs))
    sub.expect('.* ready')

    icestorm.expect('subscribeAndGetPublisher:')

    pub = Util.spawn('%s %s' %(pubCmd, pubargs))

    pub.expect('publishing tick events')
    time.sleep(3)
    sub.expect('[0-9][0-9]/[0-9][0-9].*\n[0-9][0-9]/[0-9][0-9]')

    pub.kill(signal.SIGINT)
    pub.waitTestSuccess()

    sub.kill(signal.SIGINT)
    sub.waitTestSuccess()

    if sub.hasInterruptSupport():
        icestorm.expect('unsubscribe:')
    print("ok")

def run(subCmd, pubCmd):
    sys.stdout.write("cleaning databases... ")
    sys.stdout.flush()
    Util.cleanDbDir("db")
    print("ok")

    if Util.defaultHost:
        args = ' --IceBox.Service.IceStorm="IceStormService,35:createIceStorm --Ice.Config=config.service %s"' % Util.defaultHost
    else:
        args = ''

    icestorm = Util.spawn('%s --Ice.Config=config.icebox --Ice.PrintAdapterReady %s' % (Util.getIceBox(), args))
                          
    icestorm.expect('.* ready')

    runtest(icestorm, subCmd, "", pubCmd, "")

    subargs = [" --oneway", " --twoway", " --datagram", " --twoway", " --ordered", " --batch"]
    for s in subargs:
        runtest(icestorm, subCmd, s, pubCmd, "")
    pubargs = [" --oneway", " --datagram", " --twoway" ]
    for s in pubargs:
        runtest(icestorm, subCmd, "", pubCmd, s)

    admin = Util.spawn(Util.getIceBoxAdmin() + ' --Ice.Config=config.icebox shutdown')
    admin.waitTestSuccess()
    icestorm.waitTestSuccess()
