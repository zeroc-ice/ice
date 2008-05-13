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
demoscript.Util.cleanDbDir("db/registry")
demoscript.Util.cleanDbDir("db/node")
demoscript.Util.cleanDbDir("certs")
print "ok"

if demoscript.Util.defaultHost:
    args = ' --IceGrid.Node.PropertiesOverride="Ice.Default.Host=127.0.0.1"'
else:
    args = ''

# If this is cygwin add the location of the real python to the PATH.
if demoscript.Util.isCygwin():
    os.environ["PATH"] = demoscript.Util.pythonhome + os.pathsep + os.environ["PATH"]

print "creating certificates...",
sys.stdout.flush()
makecerts = demoscript.Util.spawn('python makecerts.py')
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
print "ok"

print "starting icegrid...",
sys.stdout.flush()
registryProps = " --Ice.PrintAdapterReady" + \
                " --IceGrid.Registry.AdminSSLPermissionsVerifier=DemoIceGrid/NullSSLPermissionsVerifier"
registry = demoscript.Util.spawn('icegridregistry --Ice.Config=config.registry' + registryProps)
registry.expect('IceGrid.Registry.Internal ready\r{1,2}\nIceGrid.Registry.Server ready\r{1,2}\nIceGrid.Registry.Client ready')
node = demoscript.Util.spawn('icegridnode --Ice.Config=config.node --Ice.PrintAdapterReady %s' % (args))
node.expect('IceGrid.Node ready')
print "ok"

print "starting glacier2...",
sys.stdout.flush()

glacier2Props = " --Ice.PrintAdapterReady --Glacier2.SessionTimeout=5" + \
                " --Glacier2.SSLSessionManager=DemoIceGrid/AdminSSLSessionManager" + \
                " --Glacier2.SSLPermissionsVerifier=DemoGlacier2/NullSSLPermissionsVerifier"
glacier2 = demoscript.Util.spawn('glacier2router --Ice.Config=config.glacier2' + glacier2Props)
glacier2.expect('Glacier2.Client ready')
glacier2.expect('Glacier2.Server ready')
print "ok"

print "deploying application...",
sys.stdout.flush()
admin = demoscript.Util.spawn('icegridadmin --Ice.Config=config.admin')
admin.expect('>>>')
admin.sendline("application add application.xml")
admin.expect('>>>')
admin.sendline('exit')
admin.waitTestSuccess(timeout=120)
print "ok"

def runtest():
    client = demoscript.Util.spawn('./client')
    client.expect('==>')
    client.sendline('t')
    node.expect("SimpleServer says Hello World!")
    client.sendline('t')
    node.expect("SimpleServer says Hello World!")
    client.sendline('s')
    node.expect("detected termination of.*SimpleServer")
    client.sendline('x')

    client.waitTestSuccess(timeout=1)

print "testing client...", 
sys.stdout.flush()
runtest()
print "ok"

print "testing icegridadmin...",
sys.stdout.flush()

admin = demoscript.Util.spawn('icegridadmin --Ice.Config=config.admin --Ice.Default.Router="DemoGlacier2/router:ssl -p 4064"')
admin.expect('>>>')
admin.sendline("server list")
admin.expect('SimpleServer')
admin.expect('>>>')
admin.sendline('exit')
admin.waitTestSuccess(timeout=120)

admin = demoscript.Util.spawn('icegridadmin --Ice.Config=config.admin --ssl')
admin.expect('>>>')
admin.sendline("server list")
admin.expect('SimpleServer')
admin.expect('>>>')
admin.sendline('exit')
admin.waitTestSuccess(timeout=120)

admin = demoscript.Util.spawn('icegridadmin --Ice.Config=config.admin --ssl --Ice.Default.Router="DemoGlacier2/router:ssl -p 4064"')
admin.expect('>>>')
admin.sendline("server list")
admin.expect('SimpleServer')
admin.expect('>>>')
admin.sendline('exit')
admin.waitTestSuccess(timeout=120)

print "ok"

print "completing shutdown...", 
sys.stdout.flush()

admin = demoscript.Util.spawn('icegridadmin --Ice.Config=config.admin')
admin.expect('>>>')

admin.sendline('node shutdown Node')
admin.expect('>>>')
node.waitTestSuccess(timeout=120)

admin.sendline('registry shutdown Master')
admin.expect('>>>')
registry.waitTestSuccess()

admin.sendline('exit')
admin.waitTestSuccess(timeout=120)

glacier2.kill(signal.SIGINT)
glacier2.waitTestSuccess()

print "ok"
