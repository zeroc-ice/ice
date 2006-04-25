#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
command = server + TestUtil.clientServerOptions + " --Ice.Trace.Network=2"

print "starting server...",
serverPipe = os.popen(command)
TestUtil.getServerPid(serverPipe)
TestUtil.getAdapterReady(serverPipe)
print "ok"

router = os.path.join(toplevel, "bin", "glacier2router")
command = router + TestUtil.clientServerOptions + \
          r' --Ice.Warn.Dispatch=0' + \
          r' --Glacier2.AllowCategories="c1 c2"' + \
          r' --Glacier2.AddUserToAllowCategories="2"' + \
          r' --Glacier2.RouterIdentity="abc/def"' + \
          r' --Glacier2.AdminIdentity="ABC/DEF"' + \
          r' --Glacier2.Client.Endpoints="tcp -h 127.0.0.1 -p 12347 -t 10000:ssl -h 127.0.0.1 -p 12348 -t 10000"' + \
          r' --Glacier2.Server.Endpoints="tcp -h 127.0.0.1 -t 10000"' \
          r' --Glacier2.Admin.Endpoints="tcp -h 127.0.0.1 -p 12349 -t 10000"' + \
          r' --Glacier2.SessionManager="sessionmanager:tcp -h 127.0.0.1 -p 12350 -t 10000"' + \
          r' --Glacier2.PermissionsVerifier="verifier:tcp -h 127.0.0.1 -p 12350 -t 10000"' + \
          r" --IceSSL.DefaultDir=" + os.path.join(toplevel, "certs") + \
          r' --IceSSL.CertFile=s_rsa1024_pub.pem' + \
          r' --IceSSL.KeyFile=s_rsa1024_priv.pem' + \
          r' --IceSSL.CertAuthFile=cacert.pem' + ' --Ice.Trace.Network=2'

print "starting router...",
starterPipe = os.popen(command)
TestUtil.getServerPid(starterPipe)
TestUtil.getAdapterReady(starterPipe)
print "ok"

client = os.path.join(testdir, "client")
command = client + TestUtil.clientOptions + \
           " --IceSSL.DefaultDir=" + os.path.join(toplevel, "certs") + \
           " --IceSSL.CertFile=c_rsa1024_pub.pem" + \
           " --IceSSL.KeyFile=c_rsa1024_priv.pem" + \
           " --IceSSL.CertAuthFile=cacert.pem"

print "starting client...",
clientPipe = os.popen(command)
print "ok"

TestUtil.printOutputFromPipe(clientPipe)

clientStatus = TestUtil.closePipe(clientPipe)
serverStatus = TestUtil.closePipe(serverPipe)
starterStatus = TestUtil.closePipe(starterPipe)

if clientStatus or serverStatus or starterStatus:
    TestUtil.killServers()
    sys.exit(1)

sys.exit(0)
