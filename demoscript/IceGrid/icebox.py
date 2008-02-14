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
    node.expect('IceGrid.Registry.Internal ready')
    node.expect('IceGrid.Registry.Server ready')
    node.expect('IceGrid.Registry.AdminCallbackRouter ready')
    node.expect('IceGrid.Registry.Client ready')
    if demoscript.Util.defaultLanguage == "C++":
        node.expect('IceGrid.Registry.AdminSessionManager ready')
    node.expect('IceGrid.Node ready')
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
    node.expect("Hello from Homer")
    client.waitTestSuccess(timeout=1)

    client = demoscript.Util.spawn(clientStr)
    node.expect("Hello from Marge")
    client.waitTestSuccess(timeout=1)
    
    client = demoscript.Util.spawn(clientStr)
    node.expect("Hello from Bart")
    client.waitTestSuccess(timeout=1)

    client = demoscript.Util.spawn(clientStr)
    node.expect("Hello from Lisa")
    client.waitTestSuccess(timeout=1)

    client = demoscript.Util.spawn(clientStr)
    node.expect("Hello from Maggie")
    client.waitTestSuccess(timeout=1)
    print "ok"

    print "testing stop/start of services...", 
    sys.stdout.flush()

    admin.sendline("service stop IceBox Lisa")
    admin.expect('>>>')
    admin.sendline("service stop IceBox Maggie")
    admin.expect('>>>')

    client = demoscript.Util.spawn(clientStr)
    node.expect("Hello from Homer")
    client.waitTestSuccess(timeout=1)

    client = demoscript.Util.spawn(clientStr)
    node.expect("Hello from Marge")
    client.waitTestSuccess(timeout=1)
    
    client = demoscript.Util.spawn(clientStr)
    node.expect("Hello from Bart")
    client.waitTestSuccess(timeout=1)

    client = demoscript.Util.spawn(clientStr)
    node.expect("Hello from Homer")
    client.waitTestSuccess(timeout=1)

    admin.sendline("service start IceBox Lisa")
    admin.expect('>>>')
    admin.sendline("service start IceBox Maggie")
    admin.expect('>>>')

    admin.sendline("service stop IceBox Homer")
    admin.expect('>>>')
    admin.sendline("service stop IceBox Marge")
    admin.expect('>>>')

    client = demoscript.Util.spawn(clientStr)
    node.expect("Hello from Bart")
    client.waitTestSuccess(timeout=1)

    client = demoscript.Util.spawn(clientStr)
    node.expect("Hello from Lisa")
    client.waitTestSuccess(timeout=1)
    
    client = demoscript.Util.spawn(clientStr)
    node.expect("Hello from Maggie")
    client.waitTestSuccess(timeout=1)

    client = demoscript.Util.spawn(clientStr)
    node.expect("Hello from Bart")
    client.waitTestSuccess(timeout=1)

    print "ok"

    if demoscript.Util.defaultLanguage == "C++":
        print "testing administration with Glacier2...", 
        sys.stdout.flush()

        admin.sendline("server start DemoGlacier2")
        admin.expect('>>>')
        admin.sendline('exit')

        admin = demoscript.Util.spawn('icegridadmin --Ice.Default.Router="DemoGlacier2/router:tcp -h localhost -p 4063"', language="C++")
        admin.expect('user id:')
        admin.sendline('foo')
        admin.expect('password:')
        admin.sendline('foo')
        admin.expect('>>>')

        admin.sendline("service start IceBox Homer")
        admin.expect('>>>')
        admin.sendline("service start IceBox Marge")
        admin.expect('>>>')

        client = demoscript.Util.spawn(clientStr)
        node.expect("Hello from Lisa")
        client.waitTestSuccess(timeout=1)
    
        client = demoscript.Util.spawn(clientStr)
        node.expect("Hello from Maggie")
        client.waitTestSuccess(timeout=1)

        client = demoscript.Util.spawn(clientStr)
        node.expect("Hello from Homer")
        client.waitTestSuccess(timeout=1)

        client = demoscript.Util.spawn(clientStr)
        node.expect("Hello from Marge")
        client.waitTestSuccess(timeout=1)
    
        client = demoscript.Util.spawn(clientStr)
        node.expect("Hello from Bart")
        client.waitTestSuccess(timeout=1)

        print "ok"

    admin.sendline('registry shutdown Master')
    admin.sendline('exit')
    admin.waitTestSuccess()
    node.waitTestSuccess()
