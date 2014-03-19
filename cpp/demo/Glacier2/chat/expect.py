#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, signal

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "demoscript")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(path[0])

from demoscript import Util

server = Util.spawn('./server --Ice.PrintAdapterReady')
server.expect('.* ready')

glacier2 = Util.spawn(Util.getGlacier2Router() + ' --Ice.Config=config.glacier2 --Ice.PrintAdapterReady')
glacier2.expect('Glacier2.Client ready')
glacier2.expect('Glacier2.Server ready')

sys.stdout.write("starting client 1... ")
sys.stdout.flush()
client1 = Util.spawn('./client')
client1.expect('user id:')
client1.sendline("foo")
client1.expect('password:')
client1.sendline("foo")
print("ok")

sys.stdout.write("starting client 2... ")
sys.stdout.flush()
client2 = Util.spawn('./client')
client2.expect('user id:')
client2.sendline("bar")
client2.expect('password:')
client2.sendline("bar")

client1.expect("bar has entered the chat room")
print("ok")

sys.stdout.write("testing chat... ")
sys.stdout.flush()
client1.sendline("hi")
client1.expect("foo says: hi")
client2.expect("foo says: hi")

client2.sendline("hello")
client2.expect("bar says: hello")
client1.expect("bar says: hello")

client1.sendline("/quit")
client1.waitTestSuccess()
client2.expect("foo has left the chat room")

client2.sendline("/quit")
client2.waitTestSuccess()
print("ok")

server.kill(signal.SIGINT)
server.waitTestSuccess()

glacier2.kill(signal.SIGINT)
glacier2.waitTestSuccess()
