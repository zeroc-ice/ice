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

name = os.path.join("IceGrid", "update")
testdir = os.path.dirname(os.path.abspath(__file__))

node1Dir = os.path.join(testdir, "db", "node-1")
if not os.path.exists(node1Dir):
    os.mkdir(node1Dir)
else:
    IceGridAdmin.cleanDbDir(node1Dir)

node2Dir = os.path.join(testdir, "db", "node-2")
if not os.path.exists(node2Dir):
    os.mkdir(node2Dir)
else:
    IceGridAdmin.cleanDbDir(node2Dir)

nodeOverrideOptions = "--IceBinDir=\"" + TestUtil.getCppBinDir() + "\" --TestDir=\"" + testdir + "\"" + \
                      ' --NodePropertiesOverride="' + \
                      TestUtil.getCommandLine("", TestUtil.DriverConfig("server")).replace("--", "") + \
                      ' Ice.ServerIdleTime=0 Ice.PrintProcessId=0 Ice.PrintAdapterReady=0' + '"'

IceGridAdmin.iceGridTest(testdir, name, "", nodeOverrideOptions)
sys.exit(0)
