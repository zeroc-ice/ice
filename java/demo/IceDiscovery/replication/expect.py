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
from demoscript.IceDiscovery import replication

server1 = Util.spawn('java Server --Ice.Config=config.server1 --Ice.PrintAdapterReady --Ice.Warn.Connections=0')
server1.expect('.* ready')
server2 = Util.spawn('java Server --Ice.Config=config.server2 --Ice.PrintAdapterReady --Ice.Warn.Connections=0')
server2.expect('.* ready')
server3 = Util.spawn('java Server --Ice.Config=config.server3 --Ice.PrintAdapterReady --Ice.Warn.Connections=0')
server3.expect('.* ready')
client = Util.spawn('java Client --Ice.Warn.Connections=0')
client.expect('enter the number of iterations:')

replication.run(client, server1, server2, server3)
