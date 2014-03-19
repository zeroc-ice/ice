#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(path[0])

from demoscript import Util
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

sys.stdout.write("cleaning databases... ")
sys.stdout.flush()
Util.cleanDbDir("db/node")
Util.cleanDbDir("db/registry")
print("ok")

if Util.defaultHost:
    args = ' --IceGrid.Node.PropertiesOverride="Ice.Default.Host=127.0.0.1"'
else:
    args = ''

sys.stdout.write("starting icegridnode... ")
sys.stdout.flush()
node = Util.spawn(Util.getIceGridNode() + ' --Ice.Config=config.grid --IceGrid.Node.Output=db --IceGrid.Node.RedirectErrToOut --Ice.PrintAdapterReady %s' % (args))
node.expect('IceGrid.Registry.Server ready\nIceGrid.Registry.Client ready\nIceGrid.Node ready')
print("ok")

sys.stdout.write("deploying application... ")
sys.stdout.flush()
admin = Util.spawn(Util.getIceGridAdmin() + ' --Ice.Config=config.grid')
admin.expect('>>>')
admin.sendline("application add \'%s\'" %(desc))
admin.expect('>>>')
admin.sendline("server start DemoIceStorm-1")
admin.expect('>>>')
admin.sendline("server start DemoIceStorm-2")
admin.expect('>>>')
admin.sendline("server start DemoIceStorm-3")
admin.expect('>>>')
print("ok")

sys.stdout.write("testing pub/sub... ")
sys.stdout.flush()
sub = Util.spawn('./subscriber --Ice.PrintAdapterReady')

icestorm1 = Util.watch("db/DemoIceStorm-1.out")
icestorm2 = Util.watch("db/DemoIceStorm-2.out")
icestorm3 = Util.watch("db/DemoIceStorm-3.out")

icestorm1.expect('Election: node 1: reporting for duty in group 3:[-0-9A-Fa-f]+ with coordinator 3', timeout = 60)
icestorm2.expect('Election: node 2: reporting for duty in group 3:[-0-9A-Fa-f]+ with coordinator 3', timeout = 60)
icestorm3.expect('Election: node 3: reporting for duty in group 3:[-0-9A-Fa-f]+ as coordinator', timeout = 60)
         
icestorm3.expect('DemoIceStorm-3-IceStorm: Topic: time: subscribeAndGetPublisher: [-0-9A-Fa-f]+', timeout = 60)
icestorm1.expect('DemoIceStorm-1-IceStorm: Topic: time: add replica observer: [-0-9A-Fa-f]+', timeout = 60)
icestorm2.expect('DemoIceStorm-2-IceStorm: Topic: time: add replica observer: [-0-9A-Fa-f]+', timeout = 60)

sub.expect('.* ready')

pub = Util.spawn('./publisher')

time.sleep(3)
sub.expect('[0-9][0-9]/[0-9][0-9].*\n[0-9][0-9]/[0-9][0-9]')
print("ok")

sub.kill(signal.SIGINT)
sub.waitTestSuccess()
pub.kill(signal.SIGINT)
pub.waitTestSuccess()

admin.sendline("server stop DemoIceStorm-1")
admin.expect('>>>')
admin.sendline("server stop DemoIceStorm-2")
admin.expect('>>>')
admin.sendline("server stop DemoIceStorm-3")
admin.expect('>>>')

icestorm1.expect('DemoIceStorm-1-IceStorm: Topic: time: remove replica observer: [-0-9A-Fa-f]+')
icestorm2.expect('DemoIceStorm-2-IceStorm: Topic: time: remove replica observer: [-0-9A-Fa-f]+')
icestorm3.expect('DemoIceStorm-3-IceStorm: Topic: time: unsubscribe: [-0-9A-Fa-f]+')

icestorm1.terminate()
icestorm2.terminate()
icestorm3.terminate()

admin.sendline('registry shutdown Master')
admin.sendline('exit')
admin.waitTestSuccess()
node.waitTestSuccess()
