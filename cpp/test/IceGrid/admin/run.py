#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil, IceGridAdmin

if not TestUtil.isWin32() and os.getuid() == 0:
    sys.stdout.write("\n")
    sys.stdout.write("*** can't run test as root ***\n")
    sys.stdout.write("\n")
    sys.exit(0)

testdir = os.getcwd();

router = TestUtil.getGlacier2Router()
targets = []
if TestUtil.appverifier:
    targets = [ TestUtil.getIceGridNode(), TestUtil.getIceGridRegistry(), router]
    TestUtil.setAppVerifierSettings(targets)

registryProcs = IceGridAdmin.startIceGridRegistry(testdir)
nodeProc = IceGridAdmin.startIceGridNode(testdir)

sys.stdout.write("starting glacier2... ")
sys.stdout.flush()

args = ' --Glacier2.SessionTimeout=5' + \
       ' --Glacier2.Client.Endpoints="default -p 12347"' + \
       ' --Glacier2.Server.Endpoints="tcp -h 127.0.0.1"' \
       ' --Glacier2.SessionManager=TestIceGrid/AdminSessionManager' + \
       ' --Glacier2.PermissionsVerifier=Glacier2/NullPermissionsVerifier' + \
       ' --Glacier2.SSLSessionManager=TestIceGrid/AdminSSLSessionManager' + \
       ' --Glacier2.SSLPermissionsVerifier=Glacier2/NullSSLPermissionsVerifier' + \
       ' --Ice.Default.Locator="TestIceGrid/Locator:default -p 12010"' + \
       ' --IceSSL.VerifyPeer=1'
routerProc = TestUtil.startServer(router, args, count=2)
print("ok")

sys.stdout.write("testing login with username/password... ")
sys.stdout.flush()

# Direct registry connection with username/password
icegridadmin = TestUtil.getIceGridAdmin()
args = ' --Ice.Default.Locator="TestIceGrid/Locator:default -p 12010"' + \
       ' --IceGridAdmin.Username=demo' + \
       ' --IceGridAdmin.Password=dummy'
admin = TestUtil.startClient(icegridadmin, args, None, None, False)
admin.expect('>>> ')
admin.sendline("server list")
admin.expect('>>> ')
admin.sendline('exit')
admin.waitTestSuccess(timeout=120)

# Glacier2 connection with username/password
args = ' --Ice.Default.Router="Glacier2/router:default -p 12347"' + \
       ' --IceGridAdmin.Username=demo' + \
       ' --IceGridAdmin.Password=dummy'
admin = TestUtil.startClient(icegridadmin, args, None, None, False)
admin.expect('>>> ')
admin.sendline("server list")
admin.expect('>>> ')
admin.sendline('exit')
admin.waitTestSuccess(timeout=120)
print("ok")

if TestUtil.protocol == "ssl":

    sys.stdout.write("testing login with ssl... ")
    sys.stdout.flush()

    # Direct registry connection with SSL
    icegridadmin = TestUtil.getIceGridAdmin()
    args = ' --Ice.Default.Locator="TestIceGrid/Locator:default -p 12010" --ssl'
    admin = TestUtil.startClient(icegridadmin, args, None, None, False)
    admin.expect('>>> ')
    admin.sendline("server list")
    admin.expect('>>> ')
    admin.sendline('exit')
    admin.waitTestSuccess(timeout=120)

    # Glacier2 connection with username/password
    args = ' --Ice.Default.Router="Glacier2/router:default -p 12347" --ssl'
    admin = TestUtil.startClient(icegridadmin, args, None, None, False)
    admin.expect('>>> ')
    admin.sendline("server list")
    admin.expect('>>> ')
    admin.sendline('exit')
    admin.waitTestSuccess(timeout=120)

    print("ok")

sys.stdout.write("testing commands... ")
sys.stdout.flush()
icegridadmin = TestUtil.getIceGridAdmin()
args = ' --Ice.Default.Locator="TestIceGrid/Locator:default -p 12010"' + \
       ' --IceGridAdmin.Username=demo' + \
       ' --IceGridAdmin.Password=dummy'
admin = TestUtil.startClient(icegridadmin, args, None, None, False)
admin.expect('>>> ')
admin.sendline('application add application.xml')
admin.expect('>>> ')
admin.sendline('application list')
admin.expect('Test')
admin.sendline('application describe Test')
admin.expect('application `Test\'')
admin.expect('\{.*\}')
admin.expect('>>> ')
admin.sendline('application diff application.xml')
admin.expect('application `Test\'\n\{.*\}')
admin.expect('>>> ')
admin.sendline('application update application.xml')
admin.expect('>>> ')
admin.sendline('application patch Test')
admin.expect('>>> ')
admin.sendline('server list')
admin.expect('server')
admin.expect('>>> ')
admin.sendline('server describe server')
admin.expect('server `server\'\n\{.*\}')
admin.expect('>>> ')
admin.sendline('server start server')
admin.expect('>>> ')
admin.sendline('server state server')
admin.expect('^active \(.*\)')
admin.expect('>>> ')
admin.sendline('server pid server')
admin.expect('[0-9]+')
admin.expect('>>> ')
admin.sendline('server properties server')
admin.expect('>>> ')
admin.sendline('server property server Ice.Admin.ServerId')
admin.expect("^server")
admin.expect('>>> ')
admin.sendline('server patch server')
admin.expect('>>> ')
admin.sendline('server disable server')
admin.expect('>>> ')
admin.sendline('server enable server')
admin.expect('>>> ')
admin.sendline('adapter list')
admin.expect('TestAdapter')
admin.expect('>>> ')
admin.sendline('adapter endpoints TestAdapter')
admin.expect(['tcp', 'ssl'])
admin.expect('>>> ')
admin.sendline('object list')
admin.expect('test')
admin.expect('>>> ')
admin.sendline('object describe')
admin.expect('proxy = `.*\' type = `.*\'')
admin.expect('>>> ') 
admin.sendline('object find Test')
admin.expect('test')
admin.expect('>>> ')
admin.sendline('server stop server')
admin.expect('>>> ')
admin.sendline('application remove Test')
admin.expect('>>> ')
admin.sendline('registry list')
admin.expect('Master')
admin.expect('>>> ')
admin.sendline('registry ping Master')
admin.expect('registry is up')
admin.expect('>>> ')
admin.sendline('registry describe Master')
admin.expect('registry `Master\'\n{.*}')
admin.expect('>>> ')
admin.sendline('node list')
admin.expect('localnode')
admin.expect('>>> ')
admin.sendline('node describe localnode')
admin.expect('node `localnode\'\n{.*}')
admin.expect('>>> ')
admin.sendline('node load localnode')
admin.expect('load average.*\n')
admin.expect('>>> ')
admin.sendline('node ping localnode')
admin.expect('node is up')
admin.expect('>>> ')
admin.sendline('exit')
admin.waitTestSuccess(timeout=120)
print("ok")

# sys.stdout.write("testing icegridadmin... ")
# sys.stdout.flush()

# admin = Util.spawn('icegridadmin --Ice.Config=config.admin --Ice.Default.Router="DemoGlacier2/router:ssl -p 4064"')
# admin.expect('>>> ')
# admin.sendline("server list")
# admin.expect('SimpleServer')
# admin.expect('>>> ')
# admin.sendline('exit')
# admin.waitTestSuccess(timeout=120)

# admin = Util.spawn('icegridadmin --Ice.Config=config.admin --ssl')
# admin.expect('>>> ')
# admin.sendline("server list")
# admin.expect('SimpleServer')
# admin.expect('>>> ')
# admin.sendline('exit')
# admin.waitTestSuccess(timeout=120)

# admin = Util.spawn('icegridadmin --Ice.Config=config.admin --ssl --Ice.Default.Router="DemoGlacier2/router:ssl -p 4064"')
# admin.expect('>>> ')
# admin.sendline("server list")
# admin.expect('SimpleServer')
# admin.expect('>>> ')
# admin.sendline('exit')
# admin.waitTestSuccess(timeout=120)

# print("ok")

# sys.stdout.write("completing shutdown... ")
# sys.stdout.flush()

# admin = Util.spawn('icegridadmin --Ice.Config=config.admin')
# admin.expect('>>> ')

# admin.sendline('node shutdown Node')
# admin.expect('>>> ')
# node.waitTestSuccess(timeout=120)

# admin.sendline('registry shutdown Master')
# admin.expect('>>> ')
# registry.waitTestSuccess()

# admin.sendline('exit')
# admin.waitTestSuccess(timeout=120)

sys.stdout.write("stopping glacier2... ")
sys.stdout.flush()
routerProc.kill(signal.SIGTERM)
routerProc.waitTestSuccess()
print("ok")

IceGridAdmin.iceGridAdmin("node shutdown localnode")
IceGridAdmin.shutdownIceGridRegistry(registryProcs)
nodeProc.waitTestSuccess()

if TestUtil.appverifier:
    TestUtil.appVerifierAfterTestEnd(targets)
