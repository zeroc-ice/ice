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
import IcePackAdmin

name = os.path.join("IcePack", "deployer")
testdir = os.path.join(toplevel, "test", name)

if TestUtil.isWin32():
    os.environ["PATH"] = testdir + ";" + os.getenv("PATH", "")
elif TestUtil.isAIX():
    os.environ["LIBPATH"] = testdir + ":" + os.getenv("LIBPATH", "")
else:
    os.environ["LD_LIBRARY_PATH"] = testdir + ":" + os.getenv("LD_LIBRARY_PATH", "")
    os.environ["LD_LIBRARY_PATH_64"] = testdir + ":" + os.getenv("LD_LIBRARY_PATH_64", "")

#
# Start the client.
#
def startClient(options):

    global testdir

    fullClientOptions = TestUtil.clientOptions + \
                        " --Ice.Default.Locator=\"IcePack/Locator:default -p 12345\" " + \
                        options

    print "starting client...",
    clientPipe = os.popen(os.path.join(testdir, "client") + fullClientOptions + " 2>&1")
    print "ok"

    TestUtil.printOutputFromPipe(clientPipe)
    
    clientStatus = clientPipe.close()
    if clientStatus:
        print "failed"

#
# Start IcePack.
#
IcePackAdmin.cleanDbDir(os.path.join(testdir, "db"))
icePackRegistryThread = IcePackAdmin.startIcePackRegistry("12345", testdir)
icePackNodeThread = IcePackAdmin.startIcePackNode(testdir)

#
# Deploy the application, run the client and remove the application.
#
print "deploying application...",
IcePackAdmin.addApplication(os.path.join(testdir, "application.xml"), \
                            "ice.dir=" + toplevel + " " + "test.dir=" + testdir);
print "ok"

startClient("")

print "removing application...",
IcePackAdmin.removeApplication("test");
print "ok"    

#
# Deploy the application with some targets to test targets, run the
# client to test targets (-t options) and remove the application.
#
print "deploying application with target...",
IcePackAdmin.addApplication(os.path.join(testdir, "application.xml"), \
                            "debug test.localnode.Server1.manual ice.dir=" + toplevel + " test.dir=" + testdir)
print "ok"

startClient("-t")

print "removing application...",
IcePackAdmin.removeApplication("test");
print "ok"

#
# Shutdown IcePack.
#
IcePackAdmin.shutdownIcePackNode()
icePackNodeThread.join()
IcePackAdmin.shutdownIcePackRegistry()
icePackRegistryThread.join()

sys.exit(0)
