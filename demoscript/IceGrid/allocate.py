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
    admin.sendline("application add \'application-single.xml\'")
    admin.expect('>>>')
    print "ok"

    print "testing client...", 
    sys.stdout.flush()
    client1 = demoscript.Util.spawn(clientCmd)
    client1.expect('user id:')
    client1.sendline('foo')
    client1.expect('password:')
    client1.sendline('foo')

    client2 = demoscript.Util.spawn(clientCmd)
    client2.expect('user id:')
    client2.sendline('foo')
    client2.expect('password:')
    client2.sendline('foo')

    node.expect('activating server')
    client1.expect('==>')
    client2.expect(pexpect.TIMEOUT, timeout = 0)

    client1.sendline('t')
    node.expect('says Hello World!')
    client2.expect(pexpect.TIMEOUT, timeout = 0)
    client1.sendline('x')
    client1.expect(pexpect.EOF, timeout=1)
    assert client1.wait() == 0

    client2.expect('==>')
    client2.sendline('t')
    node.expect('says Hello World!')
    client2.sendline('s')
    node.expect('detected termination of server')
    client2.sendline('x')
    client2.expect(pexpect.EOF, timeout=1)
    assert client2.wait() == 0
    print "ok"

    print "deploying multiple...", 
    sys.stdout.flush()
    admin.sendline("application update \'application-multiple.xml\'")
    admin.expect('>>>')
    print "ok"

    print "testing client...", 
    sys.stdout.flush()
    client1 = demoscript.Util.spawn(clientCmd)
    client1.expect('user id:')
    client1.sendline('foo')
    client1.expect('password:')
    client1.sendline('foo')

    client2 = demoscript.Util.spawn(clientCmd)
    client2.expect('user id:')
    client2.sendline('foo')
    client2.expect('password:')
    client2.sendline('foo')

    client3 = demoscript.Util.spawn(clientCmd)
    client3.expect('user id:')
    client3.sendline('foo')
    client3.expect('password:')
    client3.sendline('foo')

    node.expect('activating server')
    client1.expect('==>')
    client2.expect('==>')
    client3.expect(pexpect.TIMEOUT, timeout = 0)

    client1.sendline('t')
    node.expect('says Hello World!')
    client2.sendline('t')
    node.expect('says Hello World!')
    client3.expect(pexpect.TIMEOUT, timeout = 0)

    client1.sendline('x')
    client1.expect(pexpect.EOF, timeout=1)
    assert client1.wait() == 0

    client3.expect('==>')
    client3.sendline('t')
    node.expect('says Hello World!')
    client2.sendline('t')
    node.expect('says Hello World!')

    client2.sendline('s')
    node.expect('detected termination of server')
    client2.sendline('x')
    client2.expect(pexpect.EOF, timeout=1)
    assert client2.wait() == 0

    client3.sendline('s')
    node.expect('detected termination of server')
    client3.sendline('x')
    client3.expect(pexpect.EOF, timeout=1)
    assert client3.wait() == 0

    print "ok"

    admin.sendline('registry shutdown Master')
    admin.sendline('exit')
    admin.expect(pexpect.EOF)
    assert admin.wait() == 0
    node.expect(pexpect.EOF)
    assert node.wait() == 0
