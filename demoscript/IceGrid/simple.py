#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, time, os
from demoscript import Util

def run(clientStr, desc = 'application'):
    sys.stdout.write("cleaning databases... ")
    sys.stdout.flush()
    nodeDir = os.path.join("db", "node")
    if not os.path.exists(nodeDir):
        os.mkdir(nodeDir)
    else:
        Util.cleanDbDir(nodeDir)
    regDir = os.path.join("db", "registry")
    if not os.path.exists(regDir):
        os.mkdir(regDir)
    else:
        Util.cleanDbDir(regDir)
    print("ok")

    if Util.defaultHost:
        args = ' --IceGrid.Node.PropertiesOverride="Ice.Default.Host=127.0.0.1"'
    else:
        args = ''

    sys.stdout.write("starting icegridnode... ")
    sys.stdout.flush()
    node = Util.spawn(Util.getIceGridNode() + ' --Ice.Config=config.grid --Ice.PrintAdapterReady %s' % (args))
    node.expect('IceGrid.Registry.Internal ready')
    node.expect('IceGrid.Registry.Server ready')
    node.expect('IceGrid.Registry.Client ready')
    node.expect('IceGrid.Node ready')
    print("ok")

    sys.stdout.write("deploying application... ")
    sys.stdout.flush()
    admin = Util.spawn(Util.getIceGridAdmin() + ' --Ice.Config=config.grid')
    admin.expect('>>>')
    admin.sendline("application add \'%s.xml\'" %(desc))
    admin.expect('>>>')
    print("ok")

    sys.stdout.write("testing client... ")
    sys.stdout.flush()
    client = Util.spawn(clientStr)
    client.expect('==>')
    client.sendline('t')
    node.expect("SimpleServer says Hello World!")
    client.sendline('t')
    node.expect("SimpleServer says Hello World!")
    client.sendline('s')
    node.expect("detected termination of.*SimpleServer")
    client.sendline('x')

    client.waitTestSuccess(timeout=1)
    print("ok")

    sys.stdout.write("deploying template... ")
    sys.stdout.flush()
    admin.sendline("application update \'%s_with_template.xml\'" % (desc))
    admin.expect('>>>')
    print("ok")

    sys.stdout.write("testing client... ")
    sys.stdout.flush()
    client = Util.spawn(clientStr)
    client.expect('==>')
    client.sendline('t')
    node.expect("SimpleServer-[123] says Hello World!")
    client.sendline('t')
    node.expect("SimpleServer-[123] says Hello World!")
    client.sendline('s')
    node.expect("detected termination of.*SimpleServer-[123]")
    client.sendline('x')

    client.waitTestSuccess(timeout=1)
    print("ok")

    sys.stdout.write("deploying replicated version... ")
    sys.stdout.flush()
    admin.sendline("application update \'%s_with_replication.xml\'" %(desc))
    admin.expect('>>> ')
    print("ok")

    sys.stdout.write("testing client... ")
    sys.stdout.flush()

    def testserver(which):
        admin.sendline('server start SimpleServer-%d' % which)
        admin.expect('>>> ')
        client = Util.spawn(clientStr)
        client.expect('==>')
        client.sendline('t')
        node.expect("SimpleServer-%d says Hello World!" % which)
        client.sendline('t')
        node.expect("SimpleServer-%d says Hello World!" % which)
        client.sendline('s')
        node.expect("detected termination of.*SimpleServer-%d" % which)
        client.sendline('x')
        client.waitTestSuccess(timeout=1)
    testserver(1)
    testserver(2)
    testserver(3)

    print("ok")

    admin.sendline('registry shutdown Master')
    admin.sendline('exit')
    admin.waitTestSuccess()
    node.waitTestSuccess()
