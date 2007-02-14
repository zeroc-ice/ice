#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil

name = os.path.join("Glacier2", "ssl")

testdir = os.path.join(toplevel, "test", name)
server = os.path.join(testdir, "server")
command = server + TestUtil.clientServerOptions

print "starting server...",
serverPipe = os.popen(command + " 2>&1")
TestUtil.getServerPid(serverPipe)
TestUtil.getAdapterReady(serverPipe)
print "ok"

router = os.path.join(toplevel, "bin", "glacier2router")
command = router + TestUtil.clientServerOptions + \
          r' --Ice.Warn.Dispatch=0' + \
          r' --Glacier2.AddSSLContext=1' + \
          r' --Glacier2.Client.Endpoints="tcp -h 127.0.0.1 -p 12347 -t 10000:ssl -h 127.0.0.1 -p 12348 -t 10000"' + \
          r' --Glacier2.Server.Endpoints="tcp -h 127.0.0.1 -t 10000"' \
          r' --Glacier2.Admin.Endpoints="tcp -h 127.0.0.1 -p 12349 -t 10000"' + \
          r' --Glacier2.SessionManager="sessionmanager:tcp -h 127.0.0.1 -p 12350 -t 10000"' + \
          r' --Glacier2.PermissionsVerifier="Glacier2/NullPermissionsVerifier"' + \
          r' --Glacier2.SSLSessionManager="sslsessionmanager:tcp -h 127.0.0.1 -p 12350 -t 10000"' + \
          r' --Glacier2.SSLPermissionsVerifier="sslverifier:tcp -h 127.0.0.1 -p 12350 -t 10000"' + \
          r" --Ice.Plugin.IceSSL=IceSSL:createIceSSL" + \
          r" --IceSSL.DefaultDir=" + os.path.join(toplevel, "certs") + \
          r' --IceSSL.CertFile=s_rsa1024_pub.pem' + \
          r' --IceSSL.KeyFile=s_rsa1024_priv.pem' + \
          r' --IceSSL.CertAuthFile=cacert.pem'

print "starting router...",
starterPipe = os.popen(command + " 2>&1")
TestUtil.getServerPid(starterPipe)
TestUtil.getAdapterReady(starterPipe)
print "ok"

client = os.path.join(testdir, "client")
command = client + TestUtil.clientOptions + \
           " --Ice.Plugin.IceSSL=IceSSL:createIceSSL" + \
           " --IceSSL.DefaultDir=" + os.path.join(toplevel, "certs") + \
           " --IceSSL.CertFile=c_rsa1024_pub.pem" + \
           " --IceSSL.KeyFile=c_rsa1024_priv.pem" + \
           " --IceSSL.CertAuthFile=cacert.pem"

print "starting client...",
clientPipe = os.popen(command + " 2>&1")
print "ok"

TestUtil.printOutputFromPipe(clientPipe)

clientStatus = TestUtil.closePipe(clientPipe)
if clientStatus:
    TestUtil.killServers()

if clientStatus or TestUtil.serverStatus():
    sys.exit(1)

sys.exit(0)
