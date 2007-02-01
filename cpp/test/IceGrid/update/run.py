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

name = os.path.join("IceGrid", "update")
testdir = os.path.join(toplevel, "test", name)

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

nodeOverrideOptions = "--IceDir=\"" + toplevel + "\" --TestDir=\"" + testdir + "\"" + \
                      ' --NodePropertiesOverride="' + \
                      TestUtil.clientServerOptions.replace("--", "") + \
                      ' Ice.ServerIdleTime=0 Ice.PrintProcessId=0 Ice.PrintAdapterReady=0' + '"'

IceGridAdmin.iceGridTest(name, "", nodeOverrideOptions)
sys.exit(0)
