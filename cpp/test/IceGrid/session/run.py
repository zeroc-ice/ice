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
import IceGridAdmin

if not TestUtil.isWin32() and os.getuid() == 0:
    print
    print "*** can't run test as root ***"
    print
    sys.exit(0)

name = os.path.join("IceGrid", "session")
testdir = os.path.join(toplevel, "test", name)

node1Dir = os.path.join(testdir, "db", "node-1")
if not os.path.exists(node1Dir):
    os.mkdir(node1Dir)
else:
    IceGridAdmin.cleanDbDir(node1Dir)

print "starting admin permissions verifier...",
serverCmd = os.path.join(testdir, "verifier") + TestUtil.commonServerOptions + TestUtil.defaultHost
if TestUtil.debug:
    print "(" + serverCmd + ")",
verifierPipe = os.popen(serverCmd + " 2>&1")
TestUtil.getServerPid(verifierPipe)
TestUtil.getAdapterReady(verifierPipe)
print "ok"

IceGridAdmin.registryOptions += \
                             r' --IceGrid.Registry.DynamicRegistration' + \
                             r' --IceGrid.Registry.PermissionsVerifier="ClientPermissionsVerifier"' + \
                             r' --IceGrid.Registry.AdminPermissionsVerifier="AdminPermissionsVerifier:tcp -p 12002"'+ \
                             r' --IceGrid.Registry.SSLPermissionsVerifier="SSLPermissionsVerifier"'

IceGridAdmin.iceGridTest(name, "application.xml", \
                         "--IceDir=\"" + toplevel + "\" --TestDir=\"" + testdir + "\"", \
                         '\\"properties-override=' + TestUtil.clientServerOptions.replace("--", "") + '\\"')

status = TestUtil.closePipe(verifierPipe)

sys.exit(0)
