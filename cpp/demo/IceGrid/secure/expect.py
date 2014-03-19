#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, os, signal

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
Util.cleanDbDir("db/master")
Util.cleanDbDir("db/slave")
Util.cleanDbDir("db/node")
Util.cleanDbDir("certs")
print("ok")

if Util.defaultHost:
    args = ' --IceGrid.Node.PropertiesOverride="Ice.Default.Host=127.0.0.1"'
else:
    args = ''

sys.stdout.write("creating certificates... ")
sys.stdout.flush()
makecerts = Util.spawn("python -u makecerts.py")
makecerts.expect("Do you want to keep this as the CA subject name?")
makecerts.sendline("y")
makecerts.expect("Enter the email address of the CA:")
makecerts.sendline("test@zeroc.com")
makecerts.expect("Sign the certificate?")
makecerts.sendline("y")
makecerts.expect("1 out of 1 certificate requests certified, commit?")
makecerts.sendline("y")
makecerts.expect("Sign the certificate?")
makecerts.sendline("y")
makecerts.expect("1 out of 1 certificate requests certified, commit?")
makecerts.sendline("y")
makecerts.expect("Sign the certificate?")
makecerts.sendline("y")
makecerts.expect("1 out of 1 certificate requests certified, commit?")
makecerts.sendline("y")
makecerts.expect("Sign the certificate?")
makecerts.sendline("y")
makecerts.expect("1 out of 1 certificate requests certified, commit?")
makecerts.sendline("y")
makecerts.expect("Sign the certificate?")
makecerts.sendline("y")
makecerts.expect("1 out of 1 certificate requests certified, commit?")
makecerts.sendline("y")
makecerts.waitTestSuccess()
print("ok")

sys.stdout.write("starting icegrid... ")
sys.stdout.flush()
masterProps = " --Ice.PrintAdapterReady"
master = Util.spawn(Util.getIceGridRegistry() + ' --Ice.Config=config.master' + masterProps)
master.expect('IceGrid.Registry.Internal ready')
master.expect('IceGrid.Registry.Server ready')
master.expect('IceGrid.Registry.Client ready')

slaveProps = " --Ice.PrintAdapterReady"
slave = Util.spawn(Util.getIceGridRegistry() + ' --Ice.Config=config.slave' + slaveProps)
slave.expect('IceGrid.Registry.Internal ready')
slave.expect('IceGrid.Registry.Server ready')
slave.expect('IceGrid.Registry.Client ready')

node = Util.spawn(Util.getIceGridNode() + ' --Ice.Config=config.node --Ice.PrintAdapterReady %s' % (args))
node.expect('IceGrid.Node ready')
print("ok")

sys.stdout.write("starting glacier2... ")
sys.stdout.flush()

glacier2Props = " --Ice.PrintAdapterReady --Glacier2.SessionTimeout=5"
glacier2 = Util.spawn(Util.getGlacier2Router() + ' --Ice.Config=config.glacier2' + glacier2Props)
glacier2.expect('Glacier2.Client ready')
glacier2.expect('Glacier2.Server ready')
print("ok")

sys.stdout.write("deploying application... ")
sys.stdout.flush()
admin = Util.spawn(Util.getIceGridAdmin() + ' --Ice.Config=config.admin')
admin.expect('>>>')
admin.sendline("application add application.xml")
admin.expect('>>>')
admin.sendline('exit')
admin.waitTestSuccess(timeout=120)
print("ok")

def runtest():
    client = Util.spawn('./client')
    client.expect('==>')
    client.sendline('t')
    node.expect("SimpleServer says Hello World!")
    client.sendline('t')
    node.expect("SimpleServer says Hello World!")
    client.sendline('s')
    node.expect("detected termination of.*SimpleServer")
    client.sendline('x')

    client.waitTestSuccess(timeout=1)

sys.stdout.write("testing client... ")
sys.stdout.flush()
runtest()
print("ok")

sys.stdout.write("completing shutdown... ")
sys.stdout.flush()

admin = Util.spawn(Util.getIceGridAdmin() + ' --Ice.Config=config.admin')
admin.expect('>>>')

admin.sendline('node shutdown Node')
admin.expect('>>>')
node.waitTestSuccess(timeout=120)

admin.sendline('registry shutdown Slave')
admin.expect('>>>')
slave.waitTestSuccess()

admin.sendline('registry shutdown Master')
admin.expect('>>>')
master.waitTestSuccess()

admin.sendline('exit')
admin.waitTestSuccess(timeout=120)

glacier2.kill(signal.SIGINT)
glacier2.waitTestSuccess()

print("ok")
