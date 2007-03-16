#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
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

name = os.path.join("IceE", "exceptions")
testdir = os.path.join(toplevel, "test", name)

print "tests with regular server."
classpath = os.getenv("CLASSPATH", "")
os.environ["CLASSPATH"] = os.path.join(testdir, "classes") + TestUtil.sep + classpath
TestUtil.clientServerTestWithOptions("", " --Ice.Warn.Connections=0")

print "tests with collocated server."
TestUtil.collocatedTestWithOptions(" --Ice.Warn.Dispatch=0")
sys.exit(0)
