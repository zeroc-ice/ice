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
from demoscript import Util
import Expect

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
    admin.sendline("application add \'application-single.xml\'")
    admin.expect('>>>')
    print("ok")

    sys.stdout.write("testing client... ")
    sys.stdout.flush()
    client1 = Util.spawn(clientCmd)
    client1.expect('user id:')
    client1.sendline('foo')
    client1.expect('password:')
    client1.sendline('foo')

    client2 = Util.spawn(clientCmd)
    client2.expect('user id:')
    client2.sendline('foo')
    client2.expect('password:')
    client2.sendline('foo')

    node.expect('activating server')
    client1.expect('==>')
    client2.expect(Expect.TIMEOUT, timeout = 0)

    client1.sendline('t')
    node.expect('says Hello World!')
    client2.expect(Expect.TIMEOUT, timeout = 0)
    client1.sendline('x')
    client1.waitTestSuccess(timeout=1)

    client2.expect('==>')
    client2.sendline('t')
    node.expect('says Hello World!')
    client2.sendline('s')
    node.expect('detected termination of server')
    client2.sendline('x')
    client2.waitTestSuccess(timeout=1)
    print("ok")

    sys.stdout.write("deploying multiple... ")
    sys.stdout.flush()
    admin.sendline("application update \'application-multiple.xml\'")
    admin.expect('>>>')
    print("ok")

    sys.stdout.write("testing client... ")
    sys.stdout.flush()
    client1 = Util.spawn(clientCmd)
    client1.expect('user id:')
    client1.sendline('foo')
    client1.expect('password:')
    client1.sendline('foo')

    client2 = Util.spawn(clientCmd)
    client2.expect('user id:')
    client2.sendline('foo')
    client2.expect('password:')
    client2.sendline('foo')

    client3 = Util.spawn(clientCmd)
    client3.expect('user id:')
    client3.sendline('foo')
    client3.expect('password:')
    client3.sendline('foo')

    node.expect('activating server')
    client1.expect('==>')
    client2.expect('==>')
    client3.expect(Expect.TIMEOUT, timeout = 0)

    client1.sendline('t')
    node.expect('says Hello World!')
    client2.sendline('t')
    node.expect('says Hello World!')
    client3.expect(Expect.TIMEOUT, timeout = 0)

    client1.sendline('x')
    client1.waitTestSuccess(timeout=1)

    client3.expect('==>')
    client3.sendline('t')
    node.expect('says Hello World!')
    client2.sendline('t')
    node.expect('says Hello World!')

    client2.sendline('s')
    node.expect('detected termination of server')
    client2.sendline('x')
    client2.waitTestSuccess(timeout=1)

    client3.sendline('s')
    node.expect('detected termination of server')
    client3.sendline('x')
    client3.waitTestSuccess(timeout=1)

    print("ok")

    admin.sendline('registry shutdown Master')
    admin.sendline('exit')
    admin.waitTestSuccess()
    node.waitTestSuccess()
