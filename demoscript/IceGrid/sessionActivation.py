#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import pexpect, sys, demoscript

def run(clientCmd):
    print "cleaning databases...",
    sys.stdout.flush()
    demoscript.Util.cleanDbDir("db/node")
    demoscript.Util.cleanDbDir("db/registry")
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
    client.expect(pexpect.EOF, timeout=1)
    assert client.wait() == 0
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
    client.expect(pexpect.EOF, timeout=1)
    assert client.wait() == 0
    node.expect('detected termination of server')

    print "ok"

    admin.sendline('registry shutdown Master')
    admin.sendline('exit')
    admin.expect(pexpect.EOF)
    assert admin.wait() == 0
    node.expect(pexpect.EOF)
    assert node.wait() == 0
