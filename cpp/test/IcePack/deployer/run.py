#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2001
# Mutable Realms, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
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

testdir = os.path.join(toplevel, "test", "IcePack", "deployer")


os.environ['LD_LIBRARY_PATH'] = testdir + ":" + os.environ['LD_LIBRARY_PATH']

#
# Start the client.
#
def startClient(options):

    updatedClientOptions = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel) + \
                           " --Ice.Default.Locator=\"IcePack/Locator:default -p 12346\" " + \
                           options

    print "starting client...",
    clientPipe = os.popen(os.path.join(testdir, "client") + updatedClientOptions)
    print "ok"

    for output in clientPipe.xreadlines():
        print output,
    
    clientStatus = clientPipe.close()
    if clientStatus:
        print "failed"

if TestUtil.protocol == "ssl":
    targets = "ssl"
else:
    targets = ""

#
# Start IcePack.
# 
TestUtil.cleanDbDir(os.path.join(testdir, "db/db"))
icePackPipe = IcePackAdmin.startIcePack(toplevel, "12346", testdir)

#
# Deploy the application, run the client and remove the application.
#
print "deploying application...",
IcePackAdmin.addApplication(toplevel, os.path.join(testdir, "application.xml"), targets);
print "ok"

startClient("")

print "removing application...",
IcePackAdmin.removeApplication(toplevel, os.path.join(testdir, "application.xml"));
print "ok"

#
# Deploy the application with some targets to test targets, run the
# client to test targets (-t options) and remove the application.
#
print "deploying application with target...",
IcePackAdmin.addApplication(toplevel, os.path.join(testdir, "application.xml"), targets + " debug Server1.manual");
print "ok"

startClient("-t")

print "removing application...",
IcePackAdmin.removeApplication(toplevel, os.path.join(testdir, "application.xml"));
print "ok"

#
# Shutdown IcePack.
#
IcePackAdmin.shutdownIcePack(toplevel, icePackPipe)

sys.exit(0)
