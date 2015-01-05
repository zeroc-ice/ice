#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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

def runDemo(properties, clientProperties):

    sys.stdout.write("starting icegridnodes... ")
    sys.stdout.flush()
    master = Util.spawn(Util.getIceGridRegistry() + properties + ' --Ice.Config=config.master ')
    master.expect('IceGrid.Registry.Internal ready\nIceGrid.Registry.Server ready\nIceGrid.Registry.Client ready')
    replica1 = Util.spawn(Util.getIceGridRegistry() + properties + ' --Ice.Config=config.replica1')
    replica1.expect('IceGrid.Registry.Server ready\nIceGrid.Registry.Client ready')
    replica2 = Util.spawn(Util.getIceGridRegistry() + properties + ' --Ice.Config=config.replica2')
    replica2.expect('IceGrid.Registry.Server ready\nIceGrid.Registry.Client ready')
    node1 = Util.spawn(Util.getIceGridNode() + properties + ' --Ice.Config=config.node1 %s' % (args))
    node1.expect('IceGrid.Node ready')
    node2 = Util.spawn(Util.getIceGridNode() + properties + ' --Ice.Config=config.node2 %s' % (args))
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
        client = Util.spawn('./client ' + clientProperties)
        client.expect('iterations:')
        client.sendline('5')
        client.expect('\(in ms\):')
        client.sendline('0')
        for i in range(1, 5):
            client.expect("Hello World!")
        client.sendline('x')
        client.sendline('x')

        client.waitTestSuccess(timeout=120)

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

    admin.sendline("application remove Simple")
    admin.expect('>>>')

    admin.sendline('registry shutdown Master')
    admin.expect('>>>')
    master.waitTestSuccess()

    admin.sendline('exit')
    admin.waitTestSuccess(timeout=120)
    print("ok")

print("running with Ice.Default.Locator set")
properties = ' --Ice.PrintAdapterReady --Ice.StdErr= --Ice.StdOut='
runDemo(properties, '')

print("running with IceGridDiscovery")
discovery = ' --Ice.Plugin.IceGridDiscovery=IceGrid:createIceGridDiscovery' + \
            ' --IceGridDiscovery.InstanceName=ReplicationDemoIceGrid' + \
            ' --Ice.Default.Locator='
properties = ' --Ice.PrintAdapterReady --Ice.StdErr= --Ice.StdOut= --Ice.Default.Locator='
runDemo(properties + discovery, discovery)
