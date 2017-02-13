#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys

path = [ ".", "..", "../..", "../../..", "../../../.." ]
head = os.path.dirname(sys.argv[0])
if len(head) > 0:
    path = [os.path.join(head, p) for p in path]
path = [os.path.abspath(p) for p in path if os.path.exists(os.path.join(p, "scripts", "TestUtil.py")) ]
if len(path) == 0:
    raise RuntimeError("can't find toplevel directory!")
sys.path.append(os.path.join(path[0], "scripts"))
import TestUtil, IceGridAdmin

if not TestUtil.isWin32() and os.getuid() == 0:
    sys.stdout.write("\n")
    sys.stdout.write("*** can't run test as root ***\n")
    sys.stdout.write("\n")
    sys.exit(0)

name = os.path.join("IceGrid", "session")

node1Dir = os.path.join(os.getcwd(), "db", "node-1")
if not os.path.exists(node1Dir):
    os.mkdir(node1Dir)
else:
    IceGridAdmin.cleanDbDir(node1Dir)

print("Running test with default encoding...")

sys.stdout.write("starting admin permissions verifier... ")
verifierProc = TestUtil.startServer(os.path.join(os.getcwd(), "verifier"), config=TestUtil.DriverConfig("server"))
print("ok")

IceGridAdmin.registryOptions += \
                             r' --IceGrid.Registry.DynamicRegistration' + \
                             r' --IceGrid.Registry.SessionFilters' + \
                             r' --IceGrid.Registry.AdminSessionFilters' + \
                             r' --IceGrid.Registry.PermissionsVerifier="ClientPermissionsVerifier"' + \
                             r' --IceGrid.Registry.AdminPermissionsVerifier="AdminPermissionsVerifier:tcp -p 12002"'+ \
                             r' --IceGrid.Registry.SSLPermissionsVerifier="SSLPermissionsVerifier"' + \
                             r' --IceGrid.Registry.AdminSSLPermissionsVerifier="SSLPermissionsVerifier"'

IceGridAdmin.iceGridTest("application.xml",
    '--IceBinDir="%s" --TestDir="%s"' % (TestUtil.getCppBinDir(), os.getcwd()),
    'properties-override=\'%s\'' % IceGridAdmin.iceGridNodePropertiesOverride())

verifierProc.waitTestSuccess()

print("Running test with 1.0 encoding...")

sys.stdout.write("starting admin permissions verifier... ")
verifierProc = TestUtil.startServer(os.path.join(os.getcwd(), "verifier"), config=TestUtil.DriverConfig("server"))
print("ok")

IceGridAdmin.iceGridTest("application.xml",
    '--Ice.Default.EncodingVersion=1.0 --IceBinDir="%s" --TestDir="%s"' % (TestUtil.getCppBinDir(), os.getcwd()),
    'properties-override=\'%s\'' % IceGridAdmin.iceGridNodePropertiesOverride())

verifierProc.waitTestSuccess()
