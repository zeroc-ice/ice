#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, getopt

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

if not os.environ.has_key('ICE_HOME'):
    print "ICE_HOME is not defined."
    sys.exit(0)

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil
import IceGridAdmin

try:
    opts, args = getopt.getopt(sys.argv[1:], "m")
except getopt.GetoptError:
    usage()

mono = 0
for o, a in opts:
    if o == "-m":
        mono = 1

if not TestUtil.isWin32():
    mono = 1

name = os.path.join("IceGrid", "simple")
testdir = os.path.join(toplevel, "test", name)

#
# Add locator options for the client and server. Since the server
# invokes on the locator it's also considered to be a client.
#
additionalOptions = " --Ice.Default.Locator=\"IceGrid/Locator:default -p 12010\""

IceGridAdmin.cleanDbDir(os.path.join(testdir, "db"))

#
# Start IceGrid registry.
# 
iceGridRegistryThread = IceGridAdmin.startIceGridRegistry("12010", testdir, 1)

#
# Test client/server without on demand activation.
#
additionalServerOptions=" --TestAdapter.Endpoints=default --TestAdapter.AdapterId=TestAdapter " + additionalOptions
TestUtil.mixedClientServerTestWithOptions(mono, name, additionalServerOptions, additionalOptions)

#
# Shutdown the registry.
#
IceGridAdmin.shutdownIceGridRegistry()
iceGridRegistryThread.join()

IceGridAdmin.cleanDbDir(os.path.join(testdir, "db"))

#
# Start IceGrid registry and a node.
#
iceGridRegistryThread = IceGridAdmin.startIceGridRegistry("12010", testdir, 0)
iceGridNodeThread = IceGridAdmin.startIceGridNode(testdir)

#
# Test client/server with on demand activation.
#
client = os.path.join(testdir, "client")

print "registering server with icegrid...",
if mono:
    IceGridAdmin.addApplication(os.path.join(testdir, "simple_mono_server.xml"), "test.dir=" + testdir);
else:
    IceGridAdmin.addApplication(os.path.join(testdir, "simple_server.xml"), "test.dir=" + testdir);
print "ok"

print TestUtil.createMsg(mono, "client"),
clientPipe = os.popen(TestUtil.createCmd(mono, client) + TestUtil.clientOptions + additionalOptions + \
                      " --with-deploy" + " 2>&1")
print "ok"

TestUtil.printOutputFromPipe(clientPipe)
    
clientStatus = TestUtil.closePipe(clientPipe)
if clientStatus:
    TestUtil.killServers()
    sys.exit(1)

print "unregister server with icegrid...",
IceGridAdmin.removeApplication("Test");
print "ok"
    
IceGridAdmin.shutdownIceGridNode()
iceGridNodeThread.join()
IceGridAdmin.shutdownIceGridRegistry()
iceGridRegistryThread.join()

sys.exit(0)
