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

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "demoscript")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(path[0])

from demoscript import Util

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
node.expect('IceGrid.Node ready')
print("ok")

sys.stdout.write("deploying application... ")
sys.stdout.flush()
admin = Util.spawn(Util.getIceGridAdmin() + ' --Ice.Config=config.grid')
admin.expect('>>>')
admin.sendline("application add \'application.xml\'")
admin.expect('>>>')
print("ok")

sys.stdout.write("testing preferred currency EUR... ")
sys.stdout.flush()
client = Util.spawn('./client --Ice.Default.Locator.Context.currency=EUR')
client.expect('Preferred currency configured for the client: EUR')
client.expect('Preferred currencies of the server: .*EUR.*')
client.waitTestSuccess(timeout=1)
print("ok")

sys.stdout.write("testing preferred currency GBP... ")
sys.stdout.flush()
client = Util.spawn('./client --Ice.Default.Locator.Context.currency=GBP')
client.expect('Preferred currency configured for the client: GBP')
client.expect('Preferred currencies of the server: .*GBP.*')
client.waitTestSuccess(timeout=1)
print("ok")

sys.stdout.write("testing preferred currency JPY... ")
sys.stdout.flush()
client = Util.spawn('./client --Ice.Default.Locator.Context.currency=JPY')
client.expect('Preferred currency configured for the client: JPY')
client.expect('Preferred currencies of the server: .*JPY.*')
client.waitTestSuccess(timeout=1)
print("ok")

admin.sendline('registry shutdown Master')
admin.sendline('exit')
admin.waitTestSuccess()
node.waitTestSuccess()

