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

name = os.path.join("IceGrid", "noRestartUpdate")

node1Dir = os.path.join(os.getcwd(), "db", "node-1")
if not os.path.exists(node1Dir):
    os.mkdir(node1Dir)
else:
    IceGridAdmin.cleanDbDir(node1Dir)

node2Dir = os.path.join(os.getcwd(), "db", "node-2")
if not os.path.exists(node2Dir):
    os.mkdir(node2Dir)
else:
    IceGridAdmin.cleanDbDir(node2Dir)

nodeOverrideOptions = '--IceBinDir="%s" --TestDir="%s" --NodePropertiesOverride="%s Ice.ServerIdleTime=0 Ice.PrintProcessId=0 Ice.PrintAdapterReady=0"' % (
    TestUtil.getCppBinDir("cpp"),
    os.getcwd(),
    TestUtil.getCommandLine("", TestUtil.DriverConfig("server")).replace("--", ""))

IceGridAdmin.iceGridTest("", nodeOverrideOptions)
