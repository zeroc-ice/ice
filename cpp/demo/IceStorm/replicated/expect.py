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
demoscript.Util.cleanDbDir("db/node")
demoscript.Util.cleanDbDir("db/registry")
print "ok"

if demoscript.Util.defaultHost:
    args = ' --IceGrid.Node.PropertiesOverride="Ice.Default.Host=127.0.0.1"'
else:
    args = ''

print "starting icegridnode...",
sys.stdout.flush()
node = demoscript.Util.spawn('icegridnode --Ice.Config=config.grid --Ice.PrintAdapterReady %s' % (args))
node.expect('IceGrid.Registry.Server ready\r{1,2}\nIceGrid.Registry.AdminCallbackRouter ready\r{1,2}\nIceGrid.Registry.Client ready\r{1,2}\nIceGrid.Node ready')
print "ok"

print "deploying application...",
sys.stdout.flush()
admin = demoscript.Util.spawn('icegridadmin --Ice.Config=config.grid')
admin.expect('>>>')
admin.sendline("application add \'%s\'" %(desc))
admin.expect('>>>')
print "ok"

print "testing pub/sub...",
sys.stdout.flush()
sub = demoscript.Util.spawn('./subscriber --Ice.PrintAdapterReady')

# Match each of the patterns once.
def matchpat(e, pat, timeout=60):
    matched = []
    for i in pat:
        m = e.expect(pat, timeout=timeout)
        assert not m in matched
        matched.append(m)
    assert len(matched) == len(pat)

matchpat(node, [ 'Election: node 1: reporting for duty in group 3:[-0-9A-F]+ with coordinator 3',
                 'Election: node 2: reporting for duty in group 3:[-0-9A-F]+ with coordinator 3',
                 'Election: node 3: reporting for duty in group 3:[-0-9A-F]+ as coordinator' ])
         
matchpat(node, ['DemoIceStorm-3: Topic: time: subscribeAndGetPublisher: [-0-9A-F]+',
                'DemoIceStorm-1: Topic: time: add replica observer: [-0-9A-F]+',
                'DemoIceStorm-2: Topic: time: add replica observer: [-0-9A-F]+' ])

sub.expect('.* ready')

pub = demoscript.Util.spawn('./publisher')

time.sleep(3)
sub.expect('[0-9][0-9]/[0-9][0-9].*\r{1,2}\n[0-9][0-9]/[0-9][0-9]')
print "ok"

sub.kill(signal.SIGINT)
sub.waitTestSuccess()
pub.kill(signal.SIGINT)
pub.waitTestSuccess()

matchpat(node, [ 'DemoIceStorm-1: Topic: time: remove replica observer: [-0-9A-F]+',
                 'DemoIceStorm-2: Topic: time: remove replica observer: [-0-9A-F]+' ,
                 'DemoIceStorm-3: Topic: time: unsubscribe: [-0-9A-F]+' ]) 

admin.sendline('registry shutdown Master')
admin.sendline('exit')
admin.waitTestSuccess()
node.waitTestSuccess()
