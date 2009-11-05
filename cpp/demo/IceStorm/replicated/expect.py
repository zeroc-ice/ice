#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "demoscript")) ]
if len(path) == 0:
    raise "can't find toplevel directory!"
sys.path.append(path[0])

from demoscript import *
import time, signal

desc = 'application.xml'
if Util.isNoServices() or Util.isDebugBuild():
    fi = open(desc, "r")
    desc = 'tmp_application.xml'
    fo = open(desc, "w")
    for l in fi:
        if l.find('exe="icebox"') != -1:
            if Util.isNoServices():
                l = l.replace('exe="icebox"', 'exe="' + Util.getIceBox() + '"')
            else:
                l = l.replace('exe="icebox"', 'exe="iceboxd.exe"')
        fo.write(l)
    fi.close()
    fo.close()

print "cleaning databases...",
sys.stdout.flush()
Util.cleanDbDir("db/node")
Util.cleanDbDir("db/registry")
print "ok"

if Util.defaultHost:
    args = ' --IceGrid.Node.PropertiesOverride="Ice.Default.Host=127.0.0.1"'
else:
    args = ''

print "starting icegridnode...",
sys.stdout.flush()
node = Util.spawn(Util.getIceGridNode() + ' --Ice.Config=config.grid --Ice.PrintAdapterReady %s' % (args))
node.expect('IceGrid.Registry.Server ready\nIceGrid.Registry.Client ready\nIceGrid.Node ready')
print "ok"

print "deploying application...",
sys.stdout.flush()
admin = Util.spawn(Util.getIceGridAdmin() + ' --Ice.Config=config.grid')
admin.expect('>>>')
admin.sendline("application add \'%s\'" %(desc))
admin.expect('>>>')
print "ok"

print "testing pub/sub...",
sys.stdout.flush()
sub = Util.spawn('./subscriber --Ice.PrintAdapterReady')

node.expectall([ 'Election: node 1: reporting for duty in group 3:[-0-9A-Fa-f]+ with coordinator 3',
                 'Election: node 2: reporting for duty in group 3:[-0-9A-Fa-f]+ with coordinator 3',
                 'Election: node 3: reporting for duty in group 3:[-0-9A-Fa-f]+ as coordinator' ], timeout=60)
         
node.expectall(['DemoIceStorm-3-IceStorm: Topic: time: subscribeAndGetPublisher: [-0-9A-Fa-f]+',
                'DemoIceStorm-1-IceStorm: Topic: time: add replica observer: [-0-9A-Fa-f]+',
                'DemoIceStorm-2-IceStorm: Topic: time: add replica observer: [-0-9A-Fa-f]+' ], timeout=60)

sub.expect('.* ready')

pub = Util.spawn('./publisher')

time.sleep(3)
sub.expect('[0-9][0-9]/[0-9][0-9].*\n[0-9][0-9]/[0-9][0-9]')
print "ok"

sub.kill(signal.SIGINT)
sub.waitTestSuccess()
pub.kill(signal.SIGINT)
pub.waitTestSuccess()

node.expectall([ 'DemoIceStorm-1-IceStorm: Topic: time: remove replica observer: [-0-9A-Fa-f]+',
                 'DemoIceStorm-2-IceStorm: Topic: time: remove replica observer: [-0-9A-Fa-f]+' ,
                 'DemoIceStorm-3-IceStorm: Topic: time: unsubscribe: [-0-9A-Fa-f]+' ], timeout=60) 

admin.sendline('registry shutdown Master')
admin.sendline('exit')
admin.waitTestSuccess()
node.waitTestSuccess()
