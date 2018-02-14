#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

testdir = os.getcwd();

# No need to spawn repliacs for this test.
IceGridAdmin.nreplicas = 0

def runIceGridRegistry():
    iceGrid = TestUtil.getIceGridRegistry()
    command = ' --nowarn ' + IceGridAdmin.registryOptions

    dataDir = os.path.join(testdir, "db", "registry")
    if not os.path.exists(dataDir):
        os.mkdir(dataDir)

    cmd = command + ' ' + \
            r' --Ice.ProgramName=registry' + \
            r' --IceGrid.Registry.Client.Endpoints="default -p ' + str(IceGridAdmin.iceGridPort) + '" ' + \
            r' --IceGrid.Registry.Data="' + dataDir + '"'

    driverConfig = TestUtil.DriverConfig("server")
    driverConfig.lang = "cpp"

    cmd = TestUtil.getCommandLine(iceGrid, driverConfig) + ' ' + cmd
    proc = TestUtil.spawn(cmd)
    return proc

registryProcs = IceGridAdmin.startIceGridRegistry(testdir)

sys.stdout.write("testing IceGrid file lock... ")
iceGrid = runIceGridRegistry()
iceGrid.expect(".*IceUtil::FileLockedException.*")
iceGrid.wait()
print("ok")

IceGridAdmin.shutdownIceGridRegistry(registryProcs)
