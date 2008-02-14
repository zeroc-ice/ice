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

for toplevel in [".", "..", "../..", "../../..", "../../../..", "../../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "config", "TestUtil.py")):
        break
else:
    raise "can't find toplevel directory!"

sys.path.append(os.path.join(toplevel, "config"))
import TestUtil
TestUtil.processCmdLine()

name = os.path.join("Ice", "adapterDeactivation")
testdir = os.path.dirname(os.path.abspath(__file__))
os.environ["CLASSPATH"] = os.path.join(testdir, "classes") + os.pathsep + os.getenv("CLASSPATH", "")

#
# We need to use mixedClientServerTest so that, when using SSL, the
# server-side SSL configuration properties are defined. This is
# necessary because the client creates object adapters.
#
TestUtil.mixedClientServerTest(name)
TestUtil.collocatedTest(name)
sys.exit(0)
