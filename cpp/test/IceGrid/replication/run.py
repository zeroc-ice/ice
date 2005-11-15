#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
import IceGridAdmin

name = os.path.join("IceGrid", "replication")
testdir = os.path.join(toplevel, "test", name)
client = os.path.join(testdir, "client")

if TestUtil.isWin32():
    os.environ["PATH"] = testdir + ";" + os.getenv("PATH", "")
elif TestUtil.isAIX():
    os.environ["LIBPATH"] = testdir + ":" + os.getenv("LIBPATH", "")
else:
    os.environ["LD_LIBRARY_PATH"] = testdir + ":" + os.getenv("LD_LIBRARY_PATH", "")
    os.environ["LD_LIBRARY_PATH_64"] = testdir + ":" + os.getenv("LD_LIBRARY_PATH_64", "")

if TestUtil.isWin32() and os.path.exists(os.path.join(toplevel, "bin", "iceboxd.exe")):
    iceBox = os.path.join(toplevel, "bin", "iceboxd")
else:
    iceBox = os.path.join(toplevel, "bin", "icebox")

#
# Add locator options for the client and server. Since the server
# invokes on the locator it's also considered to be a client.
#
# NOTE: We also set the retry intervals to retry several times. This
# is necessary for the test to work. The tests invoke successively on
# the same proxy and shutdown the server on each invocation. It takes
# a bit of time for the IceGrid node to detect that the server is down
# and to return a direct proxy from an active server.
#
additionalOptions = " --Ice.Default.Locator=\"IceGrid/Locator:default -p 12345\"" + \
                    " --Ice.PrintAdapterReady=0 --Ice.PrintProcessId=0 --Ice.RetryIntervals=\"0 50 100 250\""

IceGridAdmin.cleanDbDir(os.path.join(testdir, "db"))
iceGridRegistryThread = IceGridAdmin.startIceGridRegistry("12345", testdir, 0)
iceGridNodeThread = IceGridAdmin.startIceGridNode(testdir)

print "registering application with icegrid...",
IceGridAdmin.addApplication(os.path.join(testdir, "application.xml"),
                            "ice.dir=" + toplevel + " " + "test.dir=" + testdir + " icebox.exe=" + iceBox);
print "ok"

print "starting client...",
clientPipe = os.popen(client + TestUtil.clientServerOptions + additionalOptions + " 2>&1")
print "ok"

try:
    TestUtil.printOutputFromPipe(clientPipe)
except:
    pass
    
clientStatus = clientPipe.close()

print "unregister application with icegrid...",
IceGridAdmin.removeApplication("test");
print "ok"

IceGridAdmin.shutdownIceGridNode()
iceGridNodeThread.join()
IceGridAdmin.shutdownIceGridRegistry()
iceGridRegistryThread.join()

if clientStatus:
    sys.exit(1)
else:
    sys.exit(0)
