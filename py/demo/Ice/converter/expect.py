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

server = Util.spawn('Server.py --Ice.PrintAdapterReady')
server.expect('.* ready')
client = Util.spawn('Client.py')
client.expect('.*==>')

sys.stdout.write("testing with conversion... ")
sys.stdout.flush()
client.sendline('u')
if sys.version_info[0] == 2:
    server.expect('Received \\(UTF-8\\): "Bonne journ\\\\351e"')
    client.expect('Received: "Bonne journ\\\\303\\\\251e"')
else:
    server.expect('Received \\(UTF-8\\): "Bonne journ\\\\o351e"')
    client.expect('Received: "Bonne journ\\\\o303\\\\o251e"')
print("ok")

sys.stdout.write("testing without conversion... ")
sys.stdout.flush()
client.sendline('t')
if sys.version_info[0] == 2:
    server.expect('Received \\(UTF-8\\): "Bonne journ\\\\303\\\\251e"')
    client.expect('Received: "Bonne journ\\\\351e"')
else:
    server.expect('Received \\(UTF-8\\): "Bonne journ\\\\o303\\\\o251e"')
    client.expect('Received: "Bonne journ\\\\o351e"')
print("ok")

client.sendline('s')
server.waitTestSuccess()

client.sendline('x')
client.waitTestSuccess()
