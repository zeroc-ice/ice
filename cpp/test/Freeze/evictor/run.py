#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2002
# ZeroC, Inc.
# Huntsville, AL, USA
#
# All Rights Reserved
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

name = os.path.join("Freeze", "evictor")

testdir = os.path.join(toplevel, "test", name)

dbdir = os.path.join(testdir, "db")
TestUtil.cleanDbDir(dbdir)

TestUtil.clientServerTestWithOptions(toplevel, name, " " + testdir, "")
sys.exit(0)
