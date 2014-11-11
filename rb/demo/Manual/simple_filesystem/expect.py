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

server = Util.spawn('./server --Ice.PrintAdapterReady', Util.getMirrorDir("cpp"), mapping="cpp")
server.expect('.* ready')

sys.stdout.write("testing... ")
sys.stdout.flush()
client = Util.spawn('ruby Client.rb')
client.expect('Contents of root directory:\n.*Down to a sunless sea.')
client.waitTestSuccess()

server.kill(signal.SIGINT)
server.waitTestSuccess()

print("ok")
