#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2002
# ZeroC, Inc.
# Billerica, MA, USA
#
# All Rights Reserved.
#
# Ice is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 2 as published by
# the Free Software Foundation.
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

if not os.environ.has_key('ICE_HOME'):
    print "ICE_HOME is not defined."
    sys.exit(0)

ice_home = os.environ['ICE_HOME']

testdir = os.path.join(toplevel, "test", "IcePack", "deployer")


os.environ['CLASSPATH'] = os.path.join(toplevel, "lib") + TestUtil.sep + os.path.join(testdir, "classes") + \
                          TestUtil.sep + os.environ['CLASSPATH']

#
# Start the client.
#
def startClient(options):

    updatedClientOptions = TestUtil.clientOptions.replace("TOPLEVELDIR", toplevel) + \
                           " --Ice.Default.Locator=\"IcePack/Locator:default -p 12346\" " + \
                           options

    print "starting client...",

    classpath = os.path.join(toplevel, "lib") + TestUtil.sep + os.path.join(testdir, "classes") + TestUtil.sep + \
                os.getenv("CLASSPATH", "")

    clientPipe = os.popen("java -ea -classpath \"" + classpath + "\" Client " + updatedClientOptions)
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
IcePackAdmin.cleanDbDir(os.path.join(testdir, "db"))

icePackRegistryPipe = IcePackAdmin.startIcePackRegistry(ice_home, "12346", testdir)
icePackNodePipe = IcePackAdmin.startIcePackNode(ice_home, testdir)

#
# Deploy the application, run the client and remove the application.
#
print "deploying application...",
IcePackAdmin.addApplication(ice_home, os.path.join(testdir, "application.xml"), targets);
print "ok"

startClient("")

print "removing application...",
IcePackAdmin.removeApplication(ice_home, os.path.join(testdir, "application.xml"));
print "ok"

#
# Deploy the application with some targets to test targets, run the
# client to test targets (-t options) and remove the application.
#
print "deploying application with target...",
IcePackAdmin.addApplication(ice_home, os.path.join(testdir, "application.xml"),
                            targets + " debug localnode.Server1.manual");
print "ok"

startClient("-t")

print "removing application...",
IcePackAdmin.removeApplication(ice_home, os.path.join(testdir, "application.xml"));
print "ok"

#
# Shutdown IcePack.
#
IcePackAdmin.shutdownIcePackNode(ice_home, icePackNodePipe)
IcePackAdmin.shutdownIcePackRegistry(ice_home, icePackRegistryPipe)

sys.exit(0)
