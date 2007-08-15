#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import pexpect, sys, time, signal, demoscript

def runtest(icestorm, subCmd, subargs, pubCmd, pubargs):
    print "testing pub%s/sub%s..." % (pubargs, subargs),
    sys.stdout.flush()
    sub = demoscript.Util.spawn('%s --Ice.PrintAdapterReady %s' %(subCmd, subargs))
    sub.expect('.* ready')

    icestorm.expect('Subscribe:')

    pub = demoscript.Util.spawn('%s %s' %(pubCmd, pubargs))

    pub.expect('publishing tick events')
    time.sleep(3)
    sub.expect('[0-9][0-9]/[0-9][0-9].*\r{1,2}\n[0-9][0-9]/[0-9][0-9]')

    pub.kill(signal.SIGINT)
    pub.expect(pexpect.EOF)
    status = pub.wait()
    assert status == 0 or status == 130 or pub.signalstatus == signal.SIGINT

    sub.kill(signal.SIGINT)
    sub.expect(pexpect.EOF)
    status = sub.wait()
    assert status == 0 or status == 130 or sub.signalstatus == signal.SIGINT
    try:
        icestorm.expect('Unsubscribe:')
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

    icestorm = demoscript.Util.spawn('%s --Ice.Config=config.icebox --Ice.PrintAdapterReady %s' % (demoscript.Util.getIceBox(), args))
    icestorm.expect('.* ready')

    runtest(icestorm, subCmd, "", pubCmd, "")

    subargs = [" --oneway", " --twoway", " --datagram", " --twoway", " --ordered", " --batch"]
    for s in subargs:
        runtest(icestorm, subCmd, s, pubCmd, "")
    pubargs = [" --oneway", " --datagram", " --twoway" ]
    for s in pubargs:
        runtest(icestorm, subCmd, "", pubCmd, s)

    admin = demoscript.Util.spawn('iceboxadmin --Ice.Config=config.icebox shutdown')
    admin.expect(pexpect.EOF)
    assert admin.wait() == 0
    icestorm.expect(pexpect.EOF)
    assert icestorm.wait() == 0
