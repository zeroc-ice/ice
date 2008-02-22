#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
TestUtil.processCmdLine()
import IceGridAdmin

if not TestUtil.isWin32() and os.getuid() == 0:
    print
    print "*** can't run test as root ***"
    print
    sys.exit(0)

name = os.path.join("IceGrid", "session")
testdir = os.path.dirname(os.path.abspath(__file__))

node1Dir = os.path.join(testdir, "db", "node-1")
if not os.path.exists(node1Dir):
    os.mkdir(node1Dir)
else:
    IceGridAdmin.cleanDbDir(node1Dir)

print "starting admin permissions verifier...",
verifierPipe = TestUtil.startServer(os.path.join(testdir, "verifier"), " 2>&1", TestUtil.DriverConfig("server"))
TestUtil.getServerPid(verifierPipe)
TestUtil.getAdapterReady(verifierPipe)
print "ok"

IceGridAdmin.registryOptions += \
                             r' --IceGrid.Registry.DynamicRegistration' + \
                             r' --IceGrid.Registry.SessionFilters' + \
                             r' --IceGrid.Registry.AdminSessionFilters' + \
                             r' --IceGrid.Registry.PermissionsVerifier="ClientPermissionsVerifier"' + \
                             r' --IceGrid.Registry.AdminPermissionsVerifier="AdminPermissionsVerifier:tcp -p 12002"'+ \
                             r' --IceGrid.Registry.SSLPermissionsVerifier="SSLPermissionsVerifier"'

IceGridAdmin.iceGridTest(testdir, name, "application.xml", \
                         "--IceBinDir=\"" + TestUtil.getCppBinDir() + "\" --TestDir=\"" + testdir + "\"", \
                         '\\"properties-override=' + \
                         TestUtil.getCommandLine("", TestUtil.DriverConfig("server")).replace("--", "") + '\\"')

status = TestUtil.closePipe(verifierPipe)

sys.exit(0)
