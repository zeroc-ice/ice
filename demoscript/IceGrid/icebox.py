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
    if Util.getMapping() == "cpp":
        node.expect('IceGrid.Registry.AdminSessionManager ready')
    node.expect('IceGrid.Node ready')
    print("ok")

    sys.stdout.write("deploying application... ")
    sys.stdout.flush()
    admin = Util.spawn(Util.getIceGridAdmin() + ' --Ice.Config=config.grid')
    admin.expect('>>>')
    admin.sendline("application add \'%s.xml\'" %(desc))
    admin.expect('>>>')
    admin.sendline("server start IceBox")
    admin.expect('>>>', timeout=15)
    print("ok")

    sys.stdout.write("testing client... ")
    sys.stdout.flush()

    for s in [ "Homer", "Marge", "Bart", "Lisa", "Maggie" ]:
        client = Util.spawn(clientStr)
        node.expect("Hello, %s" % s)
        client.waitTestSuccess(timeout=1)

    print("ok")

    sys.stdout.write("testing stop/start of services... ")
    sys.stdout.flush()

    admin.sendline("service stop IceBox Lisa")
    admin.expect('>>>')
    admin.sendline("service stop IceBox Maggie")
    admin.expect('>>>')

    client = Util.spawn(clientStr)
    node.expect("Hello, Homer")
    client.waitTestSuccess(timeout=1)

    client = Util.spawn(clientStr)
    node.expect("Hello, Marge")
    client.waitTestSuccess(timeout=1)

    client = Util.spawn(clientStr)
    node.expect("Hello, Bart")
    client.waitTestSuccess(timeout=1)

    client = Util.spawn(clientStr)
    node.expect("Hello, Homer")
    client.waitTestSuccess(timeout=1)

    admin.sendline("service start IceBox Lisa")
    admin.expect('>>>')
    admin.sendline("service start IceBox Maggie")
    admin.expect('>>>')

    admin.sendline("service stop IceBox Homer")
    admin.expect('>>>')
    admin.sendline("service stop IceBox Marge")
    admin.expect('>>>')

    client = Util.spawn(clientStr)
    node.expect("Hello, Bart")
    client.waitTestSuccess(timeout=1)

    client = Util.spawn(clientStr)
    node.expect("Hello, Lisa")
    client.waitTestSuccess(timeout=1)

    client = Util.spawn(clientStr)
    node.expect("Hello, Maggie")
    client.waitTestSuccess(timeout=1)

    client = Util.spawn(clientStr)
    node.expect("Hello, Bart")
    client.waitTestSuccess(timeout=1)

    print("ok")

    sys.stdout.write("testing administration with Glacier2... ")
    sys.stdout.flush()

    admin.sendline("server start DemoGlacier2")
    admin.expect('>>>')
    admin.sendline('exit')

    # Windows seems to have problems with the password input.
    if Util.isWin32():
        admin = Util.spawn(Util.getIceGridAdmin() + ' --Ice.Default.Router="DemoGlacier2/router:tcp -h localhost -p 4063" -u foo -p foo')
    else:
        admin = Util.spawn(Util.getIceGridAdmin() + ' --Ice.Default.Router="DemoGlacier2/router:tcp -h localhost -p 4063"')
        admin.expect('user id:')
        admin.sendline('foo')
        admin.expect('password:')
        admin.sendline('foo')
    admin.expect('>>>', timeout=100)

    admin.sendline("service start IceBox Homer")
    admin.expect('>>>')
    admin.sendline("service start IceBox Marge")
    admin.expect('>>>')

    client = Util.spawn(clientStr)
    node.expect("Hello, Lisa")
    client.waitTestSuccess(timeout=1)

    client = Util.spawn(clientStr)
    node.expect("Hello, Maggie")
    client.waitTestSuccess(timeout=1)

    client = Util.spawn(clientStr)
    node.expect("Hello, Homer")
    client.waitTestSuccess(timeout=1)

    client = Util.spawn(clientStr)
    node.expect("Hello, Marge")
    client.waitTestSuccess(timeout=1)

    client = Util.spawn(clientStr)
    node.expect("Hello, Bart")
    client.waitTestSuccess(timeout=1)

    print("ok")

    admin.sendline('registry shutdown Master')
    admin.sendline('exit')
    admin.waitTestSuccess()
    node.waitTestSuccess()
