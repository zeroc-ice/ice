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

sys.stdout.write("cleaning databases... ")
sys.stdout.flush()
Util.cleanDbDir("db1");
Util.cleanDbDir("db2")
Util.cleanDbDir("db3")
print("ok")

if Util.defaultHost:
    a1 = ' --IceBox.Service.IceStorm="IceStormService,35:createIceStorm --Ice.Config=config.s1 %s"' \
        % Util.defaultHost
    a2 = ' --IceBox.Service.IceStorm="IceStormService,35:createIceStorm --Ice.Config=config.s2 %s"' \
        % Util.defaultHost
    a3 = ' --IceBox.Service.IceStorm="IceStormService,35:createIceStorm --Ice.Config=config.s3 %s"' \
        % Util.defaultHost
else:
    a1 = ''
    a2 = ''
    a3 = ''

sys.stdout.write("starting replicas... ")
sys.stdout.flush()
ib1 = Util.spawn('%s --Ice.Config=config.ib1 --Ice.PrintAdapterReady %s' % (Util.getIceBox(), a1))
ib1.expect('.* ready')
ib2 = Util.spawn('%s --Ice.Config=config.ib2 --Ice.PrintAdapterReady %s' % (Util.getIceBox(), a2))
ib2.expect('.* ready')
ib3 = Util.spawn('%s --Ice.Config=config.ib3 --Ice.PrintAdapterReady %s' % (Util.getIceBox(), a3))
ib3.expect('.* ready')
print("ok")

ib3.expect('Election: node 2: reporting for duty in group 2:[-0-9A-Fa-f]+ as coordinator' , timeout=20)
ib2.expect('Election: node 1: reporting for duty in group 2:[-0-9A-Fa-f]+ with coordinator 2', timeout=20)
ib1.expect('Election: node 0: reporting for duty in group 2:[-0-9A-Fa-f]+ with coordinator 2', timeout=20)

sys.stdout.write("testing pub/sub... ")
sys.stdout.flush()
sub = Util.spawn('./subscriber --Ice.PrintAdapterReady')

ib1.expect('Topic: time: add replica observer: [-0-9A-Fa-f]+')
ib2.expect('Topic: time: add replica observer: [-0-9A-Fa-f]+' )
ib3.expect('Topic: time: subscribeAndGetPublisher: [-0-9A-Fa-f]+')

sub.expect('.* ready')

pub = Util.spawn('./publisher')

time.sleep(3)
sub.expect('[0-9][0-9]/[0-9][0-9].*\n[0-9][0-9]/[0-9][0-9]')
print("ok")

sys.stdout.write("shutting down... ")
sys.stdout.flush()
sub.kill(signal.SIGINT)
sub.waitTestSuccess()
pub.kill(signal.SIGINT)
pub.waitTestSuccess()

ib1.expect('Topic: time: remove replica observer: [-0-9A-Fa-f]+')
ib2.expect('Topic: time: remove replica observer: [-0-9A-Fa-f]+')
ib3.expect('Topic: time: unsubscribe: [-0-9A-Fa-f]+')

admin = Util.spawn(Util.getIceBoxAdmin() + ' --Ice.Config=config.ib1 shutdown')
admin.waitTestSuccess()
admin = Util.spawn(Util.getIceBoxAdmin() + ' --Ice.Config=config.ib2 shutdown')
admin.waitTestSuccess()
admin = Util.spawn(Util.getIceBoxAdmin() + ' --Ice.Config=config.ib3 shutdown')
admin.waitTestSuccess()

ib1.waitTestSuccess()
ib2.waitTestSuccess()
ib3.waitTestSuccess()
print("ok")
