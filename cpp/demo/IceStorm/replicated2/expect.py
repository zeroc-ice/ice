#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os

try:
    import demoscript
except ImportError:
    for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
        toplevel = os.path.normpath(toplevel)
        if os.path.exists(os.path.join(toplevel, "demoscript")):
            break
    else:
        raise "can't find toplevel directory!"
    sys.path.append(os.path.join(toplevel))
    import demoscript

import demoscript.Util
demoscript.Util.defaultLanguage = "C++"
import time, signal

print "cleaning databases...",
sys.stdout.flush()
demoscript.Util.cleanDbDir("db1");
demoscript.Util.cleanDbDir("db2")
demoscript.Util.cleanDbDir("db3")
print "ok"

directory = os.path.dirname(os.path.abspath(__file__))
demoscript.Util.addLdPath(directory)

print "starting replicas...",
sys.stdout.flush()
ib1 = demoscript.Util.spawn('%s --Ice.Config=config.ib1 --Ice.PrintAdapterReady' % (demoscript.Util.getIceBox()))
ib1.expect('.* ready')
ib2 = demoscript.Util.spawn('%s --Ice.Config=config.ib2 --Ice.PrintAdapterReady' % (demoscript.Util.getIceBox()))
ib2.expect('.* ready')
ib3 = demoscript.Util.spawn('%s --Ice.Config=config.ib3 --Ice.PrintAdapterReady' % (demoscript.Util.getIceBox()))
ib3.expect('.* ready')
print "ok"

ib3.expect('Election: node 2: reporting for duty in group 2:[-0-9A-Fa-f]+ as coordinator' , timeout=20)
ib2.expect('Election: node 1: reporting for duty in group 2:[-0-9A-Fa-f]+ with coordinator 2', timeout=20)
ib1.expect('Election: node 0: reporting for duty in group 2:[-0-9A-Fa-f]+ with coordinator 2', timeout=20)

print "testing pub/sub...",
sys.stdout.flush()
sub = demoscript.Util.spawn('./subscriber --Ice.PrintAdapterReady')

ib1.expect('Topic: time: add replica observer: [-0-9A-Fa-f]+')
ib2.expect('Topic: time: add replica observer: [-0-9A-Fa-f]+' )
ib3.expect('Topic: time: subscribeAndGetPublisher: [-0-9A-Fa-f]+')

sub.expect('.* ready')

pub = demoscript.Util.spawn('./publisher')

time.sleep(3)
sub.expect('[0-9][0-9]/[0-9][0-9].*\r{1,2}\n[0-9][0-9]/[0-9][0-9]')
print "ok"

print "shutting down...",
sys.stdout.flush()
sub.kill(signal.SIGINT)
sub.waitTestSuccess()
pub.kill(signal.SIGINT)
pub.waitTestSuccess()

ib1.expect('Topic: time: remove replica observer: [-0-9A-Fa-f]+')
ib2.expect('Topic: time: remove replica observer: [-0-9A-Fa-f]+')
ib3.expect('Topic: time: unsubscribe: [-0-9A-Fa-f]+')

admin = demoscript.Util.spawn('iceboxadmin --Ice.Config=config.ib1 shutdown')
admin.waitTestSuccess()
admin = demoscript.Util.spawn('iceboxadmin --Ice.Config=config.ib2 shutdown')
admin.waitTestSuccess()
admin = demoscript.Util.spawn('iceboxadmin --Ice.Config=config.ib3 shutdown')
admin.waitTestSuccess()
print "ok"
