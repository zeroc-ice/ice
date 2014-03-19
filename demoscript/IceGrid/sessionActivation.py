#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys,  os
from demoscript import Util

def run(clientCmd):
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
    node.expect('IceGrid.Registry.Internal ready\nIceGrid.Registry.Server ready\nIceGrid.Registry.Client ready\nIceGrid.Node ready')
    print("ok")

    sys.stdout.write("deploying application... ")
    sys.stdout.flush()
    admin = Util.spawn(Util.getIceGridAdmin() + ' --Ice.Config=config.grid')
    admin.expect('>>>')
    admin.sendline("application add \'application.xml\'")
    admin.expect('>>>')
    print("ok")

    sys.stdout.write("testing client... ")
    sys.stdout.flush()

    client = Util.spawn(clientCmd)
    client.expect('user id:')
    client.sendline('foo')
    client.expect('password:')
    client.sendline('foo')
    node.expect('activating server')
    client.expect('==>')
    client.sendline('t')
    node.expect('says Hello World!')
    client.sendline('x')
    client.waitTestSuccess(timeout=1)
    node.expect('detected termination of server')

    client = Util.spawn(clientCmd)
    client.expect('user id:')
    client.sendline('foo')
    client.expect('password:')
    client.sendline('foo')
    node.expect('activating server')
    client.expect('==>')
    client.sendline('t')
    node.expect('says Hello World!')
    client.sendline('x')
    client.waitTestSuccess(timeout=1)
    node.expect('detected termination of server')

    print("ok")

    admin.sendline('registry shutdown Master')
    admin.sendline('exit')
    admin.waitTestSuccess()
    node.waitTestSuccess()
