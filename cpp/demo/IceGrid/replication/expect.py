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

sys.stdout.write("cleaning databases... ")
sys.stdout.flush()
Util.cleanDbDir("db/master")
Util.cleanDbDir("db/node1")
Util.cleanDbDir("db/node2")
Util.cleanDbDir("db/replica1")
Util.cleanDbDir("db/replica2")
print("ok")

if Util.defaultHost:
    args = ' --IceGrid.Node.PropertiesOverride="Ice.Default.Host=127.0.0.1"'
else:
    args = ''

sys.stdout.write("starting icegridnodes... ")
sys.stdout.flush()
master = Util.spawn(Util.getIceGridRegistry() + ' --Ice.Config=config.master --Ice.PrintAdapterReady --Ice.StdErr= --Ice.StdOut=')
master.expect('IceGrid.Registry.Internal ready\nIceGrid.Registry.Server ready\nIceGrid.Registry.Client ready')
replica1 = Util.spawn(Util.getIceGridRegistry() + ' --Ice.Config=config.replica1 --Ice.PrintAdapterReady --Ice.StdErr= --Ice.StdOut=')
replica1.expect('IceGrid.Registry.Server ready\nIceGrid.Registry.Client ready')
replica2 = Util.spawn(Util.getIceGridRegistry() + ' --Ice.Config=config.replica2 --Ice.PrintAdapterReady --Ice.StdErr= --Ice.StdOut=')
replica2.expect('IceGrid.Registry.Server ready\nIceGrid.Registry.Client ready')
node1 = Util.spawn(Util.getIceGridNode() + ' --Ice.Config=config.node1 --Ice.PrintAdapterReady --Ice.StdErr= --Ice.StdOut= %s' % (args))
node1.expect('IceGrid.Node ready')
node2 = Util.spawn(Util.getIceGridNode() + ' --Ice.Config=config.node2 --Ice.PrintAdapterReady --Ice.StdErr= --Ice.StdOut= %s' % (args))
node2.expect('IceGrid.Node ready')
print("ok")

sys.stdout.write("deploying application... ")
sys.stdout.flush()
admin = Util.spawn(Util.getIceGridAdmin() + ' --Ice.Config=config.client')
admin.expect('>>>')
admin.sendline("application add \'application.xml\'")
admin.expect('>>>')
print("ok")

def runtest():
    client = Util.spawn('./client')
    client.expect('iterations:')
    client.sendline('5')
    client.expect('\(in ms\):')
    client.sendline('0')
    for i in range(1, 5):
        client.expect("Hello World!")
    client.sendline('x')
    client.sendline('x')

    client.waitTestSuccess(timeout=1)

sys.stdout.write("testing client... ")
sys.stdout.flush()
runtest()
print("ok")

sys.stdout.write("testing replication... ")
sys.stdout.flush()
admin.sendline('registry shutdown Replica1')
admin.expect('>>>')
replica1.waitTestSuccess()
runtest()
admin.sendline('registry shutdown Replica2')
admin.expect('>>>')
replica2.waitTestSuccess()
runtest()
print("ok")

sys.stdout.write("completing shutdown... ")
sys.stdout.flush()
admin.sendline('node shutdown node1')
admin.expect('>>>')
node1.waitTestSuccess(timeout=120)

admin.sendline('node shutdown node2')
admin.expect('>>>')
node2.waitTestSuccess(timeout=120)

admin.sendline('registry shutdown Master')
admin.expect('>>>')
master.waitTestSuccess()

admin.sendline('exit')
admin.waitTestSuccess(timeout=120)
print("ok")
