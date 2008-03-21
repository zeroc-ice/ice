#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, demoscript, os

def run(clientCmd):
    print "cleaning databases...",
    sys.stdout.flush()
    nodeDir = os.path.join("db", "node")
    if not os.path.exists(nodeDir):
        os.mkdir(nodeDir)
    else:
        demoscript.Util.cleanDbDir(nodeDir)
    regDir = os.path.join("db", "registry")
    if not os.path.exists(regDir):
        os.mkdir(regDir)
    else:
        demoscript.Util.cleanDbDir(regDir)
    print "ok"

    if demoscript.Util.defaultHost:
        args = ' --IceGrid.Node.PropertiesOverride="Ice.Default.Host=127.0.0.1"'
    else:
        args = ''

    print "starting icegridnode...",
    sys.stdout.flush()
    node = demoscript.Util.spawn('icegridnode --Ice.Config=config.grid --Ice.PrintAdapterReady %s' % (args))
    node.expect('IceGrid.Registry.Internal ready\r{1,2}\nIceGrid.Registry.Server ready\r{1,2}\nIceGrid.Registry.Client ready\r{1,2}\nIceGrid.Node ready')
    print "ok"

    print "deploying application...",
    sys.stdout.flush()
    admin = demoscript.Util.spawn('icegridadmin --Ice.Config=config.grid')
    admin.expect('>>>')
    admin.sendline("application add \'application.xml\'")
    admin.expect('>>>')
    print "ok"

    print "testing client...", 
    sys.stdout.flush()

    client = demoscript.Util.spawn(clientCmd)
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

    client = demoscript.Util.spawn(clientCmd)
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

    print "ok"

    admin.sendline('registry shutdown Master')
    admin.sendline('exit')
    admin.waitTestSuccess()
    node.waitTestSuccess()
