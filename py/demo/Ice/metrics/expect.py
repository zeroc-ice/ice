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
sys.path.append(os.path.join(path[0], "scripts"))

from demoscript import Util
from demoscript.Ice import async

cwd = os.getcwd();

os.chdir("../hello")
server = Util.spawn('Server.py --Ice.Admin.Endpoints="tcp -p 10002" --Ice.PrintAdapterReady')
server.expect('.* ready')

client = Util.spawn('Client.py --Ice.Admin.Endpoints="tcp -p 10003"')
client.expect('.*==>')
os.chdir(cwd)

sys.stdout.write("testing server metrics dump... ")
sys.stdout.flush()

metrics = Util.spawn('Metrics.py --Endpoints="tcp -p 10002" --InstanceName=server dump')
metrics.expect("View: ByParent")
metrics.expect("View: Debug")
metrics.waitTestSuccess()

client.sendline('t')
metrics = Util.spawn('Metrics.py --Endpoints="tcp -p 10002" --InstanceName=server dump Debug Dispatch')
metrics.expect("hello\s\[sayHello\]\s*\|\s*0\|\s*1\|.*\|")
metrics.waitTestSuccess()

client.sendline('t')
metrics = Util.spawn('Metrics.py --Endpoints="tcp -p 10002" --InstanceName=server dump Debug Dispatch')
metrics.expect("hello\s\[sayHello\]\s*\|\s*0\|\s*2\|.*\|")
metrics.waitTestSuccess()

print("ok")


sys.stdout.write("testing client metrics dump... ")
sys.stdout.flush()

metrics = Util.spawn('Metrics.py --Endpoints="tcp -p 10003" --InstanceName=client dump')
metrics.expect("View: ByParent")
metrics.expect("View: Debug")
metrics.waitTestSuccess()

client.sendline('t')
metrics = Util.spawn('Metrics.py --Endpoints="tcp -p 10003" --InstanceName=client dump Debug Invocation')
metrics.expect("hello -t -e 1.1 \[sayHello\]\s*\|\s*0\|\s*3\|.*")
metrics.waitTestSuccess()

client.sendline('t')
metrics = Util.spawn('Metrics.py --Endpoints="tcp -p 10003" --InstanceName=client dump Debug Invocation')
metrics.expect("hello -t -e 1.1 \[sayHello\]\s*\|\s*0\|\s*4\|.*")
metrics.waitTestSuccess()

client.sendline('o')
metrics = Util.spawn('Metrics.py --Endpoints="tcp -p 10003" --InstanceName=client dump Debug Invocation')
metrics.expect("hello -o -e 1.1 \[sayHello\]\s*\|\s*0\|\s*1\|.*")
metrics.waitTestSuccess()

print("ok")

sys.stdout.write("testing view enable/disable... ")
sys.stdout.flush()

metrics = Util.spawn('Metrics.py --Endpoints="tcp -p 10002" --InstanceName=server disable ByParent')
metrics.waitTestSuccess()

metrics = Util.spawn('Metrics.py --Endpoints="tcp -p 10002" --InstanceName=server dump')
metrics.expect("View: ([a-zA-Z]*)")
assert metrics.match.group(1).strip() == "Debug"
metrics.waitTestSuccess()

metrics = Util.spawn('Metrics.py --Endpoints="tcp -p 10002" --InstanceName=server enable ByParent')
metrics.waitTestSuccess()

metrics = Util.spawn('Metrics.py --Endpoints="tcp -p 10002" --InstanceName=server dump')
metrics.expect("View: ([a-zA-Z]*)")
assert metrics.match.group(1).strip() == "ByParent"
metrics.waitTestSuccess()

print ("ok")

client.sendline('s')
server.waitTestSuccess()

client.sendline('x')
client.waitTestSuccess()


