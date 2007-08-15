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

def run(clientStr):
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
    client = demoscript.Util.spawn(clientStr)
    client.expect('==>')
    client.sendline('t')
    node.expect("SimpleServer says Hello World!")
    client.sendline('t')
    node.expect("SimpleServer says Hello World!")
    client.sendline('s')
    node.expect("detected termination of.*SimpleServer")
    client.sendline('x')

    client.expect(pexpect.EOF, timeout=1)
    assert client.wait() == 0
    print "ok"

    print "deploying template...", 
    sys.stdout.flush()
    admin.sendline("application update \'application_with_template.xml\'")
    admin.expect('>>>')
    print "ok"

    print "testing client...", 
    sys.stdout.flush()
    client = demoscript.Util.spawn(clientStr)
    client.expect('==>')
    client.sendline('t')
    node.expect("SimpleServer-[123] says Hello World!")
    client.sendline('t')
    node.expect("SimpleServer-[123] says Hello World!")
    client.sendline('s')
    node.expect("detected termination of.*SimpleServer-[123]")
    client.sendline('x')

    client.expect(pexpect.EOF, timeout=1)
    assert client.wait() == 0
    print "ok"

    print "deploying replicated version...", 
    sys.stdout.flush()
    admin.sendline("application update \'application_with_replication.xml\'")
    admin.expect('>>> ')
    print "ok"

    print "testing client...", 
    sys.stdout.flush()

    client = demoscript.Util.spawn(clientStr + ' --Ice.Default.Host=127.0.0.1')
    client.expect('==>')
    client.sendline('t')
    node.expect("SimpleServer-1 says Hello World!")
    client.sendline('t')
    node.expect("SimpleServer-1 says Hello World!")
    client.sendline('s')
    node.expect("detected termination of.*SimpleServer-1")
    client.sendline('x')
    client.expect(pexpect.EOF, timeout=1)
    assert client.wait() == 0

    client = demoscript.Util.spawn(clientStr + ' --Ice.Default.Host=127.0.0.1')
    client.expect('==>')
    client.sendline('t')
    node.expect("SimpleServer-2 says Hello World!")
    client.sendline('t')
    node.expect("SimpleServer-2 says Hello World!")
    client.sendline('s')
    node.expect("detected termination of.*SimpleServer-2")
    client.sendline('x')
    client.expect(pexpect.EOF, timeout=1)
    assert client.wait() == 0

    client = demoscript.Util.spawn(clientStr + ' --Ice.Default.Host=127.0.0.1')
    client.expect('==>')
    client.sendline('t')
    node.expect("SimpleServer-3 says Hello World!")
    client.sendline('t')
    node.expect("SimpleServer-3 says Hello World!")
    client.sendline('s')
    node.expect("detected termination of.*SimpleServer-3")
    client.sendline('x')
    client.expect(pexpect.EOF, timeout=1)
    assert client.wait() == 0

    print "ok"

    admin.sendline('registry shutdown Master')
    admin.sendline('exit')
    admin.expect(pexpect.EOF)
    assert admin.wait() == 0
    node.expect(pexpect.EOF)
    assert node.wait() == 0
