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
import IceGridAdmin

name = os.path.join("IceGrid", "session")
testdir = os.path.join(toplevel, "test", name)

node1Dir = os.path.join(testdir, "db", "node-1")
if not os.path.exists(node1Dir):
    os.mkdir(node1Dir)
else:
    IceGridAdmin.cleanDbDir(node1Dir)

IceGridAdmin.registryOptions += \
                             r' --IceGrid.Registry.DynamicRegistration' + \
                             r' --IceGrid.Registry.PermissionsVerifier="ClientPermissionsVerifier"' + \
                             r' --IceGrid.Registry.AdminPermissionsVerifier="AdminPermissionsVerifier"' + \
                             r' --IceGrid.Registry.SSLPermissionsVerifier="SSLPermissionsVerifier"' + \
                             r' --IceGrid.Registry.AdminSSLPermissionsVerifier="SSLPermissionsVerifier"'

IceGridAdmin.iceGridTest(name, "application.xml", \
                         "--IceDir=\"" + toplevel + "\" --TestDir=\"" + testdir + "\"", \
                         '\\"properties-override=' + TestUtil.clientServerOptions.replace("--", "") + '\\"')
sys.exit(0)
