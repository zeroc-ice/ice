#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
    raise "can't find toplevel directory!"
sys.path.append(os.path.join(path[0]))
from scripts import *

def runIceGridRegistry(testdir):
    iceGrid = ""
    if TestUtil.isBCC2010() or TestUtil.isVC6():
        iceGrid = os.path.join(TestUtil.getServiceDir(), "icegridregistry")
    else:
        iceGrid = os.path.join(TestUtil.getCppBinDir(), "icegridregistry")

    command = ' --nowarn ' + IceGridAdmin.registryOptions

    dataDir = os.path.join(testdir, "db")
    if not os.path.exists(dataDir):
        os.mkdir(dataDir)

    cmd = command + ' ' + TestUtil.getQtSqlOptions('IceGrid') + \
            r' --Ice.ProgramName=registry' + \
            r' --IceGrid.Registry.Client.Endpoints="default -p ' + str(IceGridAdmin.iceGridPort) + '" ' + \
            r' --IceGrid.Registry.Data=' + dataDir

    driverConfig = TestUtil.DriverConfig("server")
    driverConfig.lang = "cpp"

    cmd = TestUtil.getCommandLine(iceGrid, driverConfig) + ' ' + cmd
    proc = TestUtil.spawn(cmd)
    return proc

IceGridAdmin.cleanDbDir("./db")

print "testing IceGrid file lock...",
iceGrid1 = runIceGridRegistry(".")
iceGrid1.expect("[^\n]+ ready\n")

iceGrid2 = runIceGridRegistry(".")
iceGrid2.expect(".*IceUtil::FileLockedException.*")
print "ok"

IceGridAdmin.iceGridAdmin("registry shutdown")
IceGridAdmin.cleanDbDir("./db")
