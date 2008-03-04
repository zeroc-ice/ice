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

desc = 'application.xml'
if demoscript.Util.mode == 'debug':
    fi = open(desc, "r")
    desc = 'tmp_application.xml'
    fo = open(desc, "w")
    for l in fi:
        if l.find('exe="icebox"'):
            l = l.replace('exe="icebox"', 'exe="iceboxd.exe"')
        fo.write(l)
    fi.close()
    fo.close()

print "cleaning databases...",
sys.stdout.flush()
demoscript.Util.cleanDbDir("db1");
demoscript.Util.cleanDbDir("db2")
demoscript.Util.cleanDbDir("db3")
print "ok"

if demoscript.Util.defaultHost:
    args = ' --IceGrid.Node.PropertiesOverride="Ice.Default.Host=127.0.0.1"'
else:
    args = ''

directory = os.path.dirname(os.path.abspath(__file__))
demoscript.Util.addLdPath(directory)

print "starting replicas...",
sys.stdout.flush()
ib1 = demoscript.Util.spawn('%s --Ice.Config=config.ib1 --Ice.PrintAdapterReady %s' % (demoscript.Util.getIceBox(), args))
ib1.expect('.* ready')
ib2 = demoscript.Util.spawn('%s --Ice.Config=config.ib2 --Ice.PrintAdapterReady %s' % (demoscript.Util.getIceBox(), args))
ib2.expect('.* ready')
ib3 = demoscript.Util.spawn('%s --Ice.Config=config.ib3 --Ice.PrintAdapterReady %s' % (demoscript.Util.getIceBox(), args))
ib3.expect('.* ready')
print "ok"

ib3.expect('Election: node 2: reporting for duty in group 2:[-0-9A-F]+ as coordinator' , timeout=20)
ib2.expect('Election: node 1: reporting for duty in group 2:[-0-9A-F]+ with coordinator 2', timeout=20)
ib1.expect('Election: node 0: reporting for duty in group 2:[-0-9A-F]+ with coordinator 2', timeout=20)

print "testing pub/sub...",
sys.stdout.flush()
sub = demoscript.Util.spawn('./subscriber --Ice.PrintAdapterReady')

ib1.expect('Topic: time: add replica observer: [-0-9A-F]+')
ib2.expect('Topic: time: add replica observer: [-0-9A-F]+' )
ib3.expect('Topic: time: subscribeAndGetPublisher: [-0-9A-F]+')

sub.expect('.* ready')

pub = demoscript.Util.spawn('./publisher')

time.sleep(3)
sub.expect('[0-9][0-9]/[0-9][0-9].*\r{1,2}\n[0-9][0-9]/[0-9][0-9]')
print "ok"

sub.kill(signal.SIGINT)
sub.waitTestSuccess()
pub.kill(signal.SIGINT)
pub.waitTestSuccess()

ib1.expect('Topic: time: remove replica observer: [-0-9A-F]+')
ib2.expect('Topic: time: remove replica observer: [-0-9A-F]+' )
ib3.expect('Topic: time: unsubscribe: [-0-9A-F]+')

admin = demoscript.Util.spawn('iceboxadmin --Ice.Config=config.ib1 shutdown')
admin.waitTestSuccess()
admin = demoscript.Util.spawn('iceboxadmin --Ice.Config=config.ib2 shutdown')
admin.waitTestSuccess()
admin = demoscript.Util.spawn('iceboxadmin --Ice.Config=config.ib3 shutdown')
admin.waitTestSuccess()
