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

name = os.path.join("IceGrid", "deployer")
testdir = os.path.join(toplevel, "test", name)

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
# Start the client.
#
def startClient(options):

    global testdir

    fullClientOptions = TestUtil.clientOptions + \
                        " --Ice.Default.Locator=\"IceGrid/Locator:default -p 12345\" " + \
                        options

    print "starting client...",
    clientPipe = os.popen(os.path.join(testdir, "client") + fullClientOptions + " 2>&1")
    print "ok"

    TestUtil.printOutputFromPipe(clientPipe)
    
    clientStatus = clientPipe.close()
    return clientStatus

#
# Start IceGrid.
#
IceGridAdmin.cleanDbDir(os.path.join(testdir, "db"))
iceGridRegistryThread = IceGridAdmin.startIceGridRegistry("12345", testdir, 0)
iceGridNodeThread = IceGridAdmin.startIceGridNode(testdir)

#
# Deploy the application, run the client and remove the application.
#
print "deploying application...",
IceGridAdmin.addApplication(os.path.join(testdir, "application.xml"), \
                            "ice.dir=" + toplevel + " test.dir=" + testdir + " icebox.exe=" + iceBox);
print "ok"

status = startClient("")

print "removing application...",
IceGridAdmin.removeApplication("test");
print "ok"    

if status:
    IceGridAdmin.shutdownIceGridNode()
    iceGridNodeThread.join()
    IceGridAdmin.shutdownIceGridRegistry()
    iceGridRegistryThread.join()
    sys.exit(1)
    

#
# Deploy the application with some targets to test targets, run the
# client to test targets (-t options) and remove the application.
#
print "deploying application with target...",
IceGridAdmin.addApplication(os.path.join(testdir, "application.xml"), \
                            "moreservers moreservices moreproperties ice.dir=" + toplevel + " test.dir=" + testdir + \
			    " icebox.exe=" + iceBox)
print "ok"

status = startClient("-t")

print "removing application...",
IceGridAdmin.removeApplication("test");
print "ok"

#
# Shutdown IceGrid.
#
IceGridAdmin.shutdownIceGridNode()
iceGridNodeThread.join()
IceGridAdmin.shutdownIceGridRegistry()
iceGridRegistryThread.join()

if status:
    sys.exit(1)
else:
    sys.exit(0)
