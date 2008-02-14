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

name = os.path.join("IceBox", "configuration")
testdir = os.path.dirname(os.path.abspath(__file__))
icebox = TestUtil.getIceBox(testdir);

TestUtil.addLdPath(testdir)

cwd = os.getcwd()
os.chdir(testdir)

TestUtil.clientServerTestWithOptionsAndNames(name, "--Ice.Config=config.icebox", "", icebox, "client")
TestUtil.clientServerTestWithOptionsAndNames(name, "--Ice.Config=config.icebox2", "", icebox, "client")

os.chdir(cwd)

sys.exit(0)
