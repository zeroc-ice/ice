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

if Util.isDarwin():
    print("This demo is not supported under OS X.")
    sys.exit(0)

sys.stdout.write("cleaning databases... ")
sys.stdout.flush()
Util.cleanDbDir("db")
print("ok")

print("testing IceUtil::Cache evictor")
server = Util.spawn('./server --Ice.PrintAdapterReady')
server.expect(".* ready", timeout=120)

client = Util.spawn('./client')
client.waitTestSuccess(timeout=8 * 60)
print(client.before)

server.kill(signal.SIGINT)
server.waitTestSuccess(timeout=60)

print("testing simple evictor")
server = Util.spawn('./server simple --Ice.PrintAdapterReady')
server.expect(".* ready")

client = Util.spawn('./client')
client.waitTestSuccess(timeout=8*60)
print(client.before)

server.kill(signal.SIGINT)
server.waitTestSuccess(timeout=60)
