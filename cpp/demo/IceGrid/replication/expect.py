#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os

try:
    import demoscript
except ImportError:
    for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
        toplevel = os.path.normpath(toplevel)
        if os.path.exists(os.path.join(toplevel, "demoscript")):
            break
    else:
        raise "can't find toplevel directory!"
    sys.path.append(os.path.join(toplevel))
    import demoscript

import demoscript.Util
demoscript.Util.defaultLanguage = "C++"

import signal

print "cleaning databases...",
sys.stdout.flush()
demoscript.Util.cleanDbDir("db/master")
demoscript.Util.cleanDbDir("db/node1")
demoscript.Util.cleanDbDir("db/node2")
demoscript.Util.cleanDbDir("db/replica1")
demoscript.Util.cleanDbDir("db/replica2")
print "ok"

if demoscript.Util.defaultHost:
    args = ' --IceGrid.Node.PropertiesOverride="Ice.Default.Host=127.0.0.1"'
else:
    args = ''

print "starting icegridnodes...",
sys.stdout.flush()
master = demoscript.Util.spawn('icegridregistry --Ice.Config=config.master --Ice.PrintAdapterReady --Ice.StdErr= --Ice.StdOut=')
master.expect('IceGrid.Registry.Internal ready\r{1,2}\nIceGrid.Registry.Server ready\r{1,2}\nIceGrid.Registry.Client ready')
replica1 = demoscript.Util.spawn('icegridregistry --Ice.Config=config.replica1 --Ice.PrintAdapterReady --Ice.StdErr= --Ice.StdOut=')
replica1.expect('IceGrid.Registry.Server ready\r{1,2}\nIceGrid.Registry.Client ready')
replica2 = demoscript.Util.spawn('icegridregistry --Ice.Config=config.replica2 --Ice.PrintAdapterReady --Ice.StdErr= --Ice.StdOut=')
replica2.expect('IceGrid.Registry.Server ready\r{1,2}\nIceGrid.Registry.Client ready')
node1 = demoscript.Util.spawn('icegridnode --Ice.Config=config.node1 --Ice.PrintAdapterReady --Ice.StdErr= --Ice.StdOut= %s' % (args))
node1.expect('IceGrid.Node ready')
node2 = demoscript.Util.spawn('icegridnode --Ice.Config=config.node2 --Ice.PrintAdapterReady --Ice.StdErr= --Ice.StdOut= %s' % (args))
node2.expect('IceGrid.Node ready')
print "ok"

print "deploying application...",
sys.stdout.flush()
admin = demoscript.Util.spawn('icegridadmin --Ice.Config=config.client')
admin.expect('>>>')
admin.sendline("application add \'application.xml\'")
admin.expect('>>>')
print "ok"

def runtest():
    client = demoscript.Util.spawn('./client')
    client.expect('iterations:')
    client.sendline('5')
    client.expect('\(in ms\):')
    client.sendline('0')
    for i in range(1, 5):
        client.expect("Hello World!")
    client.sendline('x')
    client.sendline('x')

    client.waitTestSuccess(timeout=1)

print "testing client...", 
sys.stdout.flush()
runtest()
print "ok"

print "testing replication...", 
sys.stdout.flush()
admin.sendline('registry shutdown Replica1')
admin.expect('>>>')
replica1.waitTestSuccess()
runtest()
admin.sendline('registry shutdown Replica2')
admin.expect('>>>')
replica2.waitTestSuccess()
runtest()
print "ok"

admin.sendline('node shutdown node1')
admin.expect('>>>')
node1.waitTestSuccess(timeout=120)

admin.sendline('node shutdown node2')
admin.expect('>>>')
node2.waitTestSuccess(timeout=120)

admin.sendline('registry shutdown Master')
admin.expect('>>>')
master.waitTestSuccess()

admin.sendline('exit')
admin.waitTestSuccess()
