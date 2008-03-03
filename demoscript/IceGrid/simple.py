#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, demoscript, time

def run(clientStr, desc = 'application'):
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
    node = demoscript.Util.spawn('icegridnode --Ice.Config=config.grid --Ice.PrintAdapterReady %s' % (args),
                                 language="C++")
    node.expect('IceGrid.Registry.Internal ready\r{1,2}\nIceGrid.Registry.Server ready\r{1,2}\nIceGrid.Registry.Client ready\r{1,2}\nIceGrid.Node ready')
    print "ok"

    print "deploying application...",
    sys.stdout.flush()
    admin = demoscript.Util.spawn('icegridadmin --Ice.Config=config.grid', language="C++")
    admin.expect('>>>')
    admin.sendline("application add \'%s.xml\'" %(desc))
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

    client.waitTestSuccess(timeout=1)
    print "ok"

    print "deploying template...", 
    sys.stdout.flush()
    admin.sendline("application update \'%s_with_template.xml\'" % (desc))
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

    client.waitTestSuccess(timeout=1)
    print "ok"

    print "deploying replicated version...", 
    sys.stdout.flush()
    admin.sendline("application update \'%s_with_replication.xml\'" %(desc))
    admin.expect('>>> ')
    print "ok"

    print "testing client...", 
    sys.stdout.flush()

    def testserver(which):
        admin.sendline('server start SimpleServer-%d' % which)
        admin.expect('>>> ')
        client = demoscript.Util.spawn(clientStr)
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

    print "ok"

    admin.sendline('registry shutdown Master')
    admin.sendline('exit')
    admin.waitTestSuccess()
    node.waitTestSuccess()
