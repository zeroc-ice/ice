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
from demoscript.Freeze import phonebook

sys.stdout.write("cleaning databases... ")
sys.stdout.flush()
Util.cleanDbDir("db")
print("ok")

server = Util.spawn('java Server --Ice.PrintAdapterReady --Freeze.Trace.Evictor=0 --Freeze.Trace.DbEnv=0')
server.expect('.* ready')
client = Util.spawn('java Client')
client.expect('>>> ')

phonebook.run(client, server)

print("running with collocated server")

sys.stdout.write("cleaning databases... ")
sys.stdout.flush()
Util.cleanDbDir("db")
print("ok")

server = Util.spawn('java Collocated --Freeze.Trace.Evictor=0 --Freeze.Trace.DbEnv=0')
server.expect('>>> ')

phonebook.run(server, server)
