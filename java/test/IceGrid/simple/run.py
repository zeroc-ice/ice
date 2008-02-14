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
import IceGridAdmin

testdir = os.path.dirname(os.path.abspath(__file__))

name = os.path.join("IceGrid", "simple")
os.environ["CLASSPATH"] = os.path.join(testdir, "classes") + os.pathsep + os.getenv("CLASSPATH", "")

#
# Test client/server without on demand activation.
#
IceGridAdmin.iceGridClientServerTest(testdir, name, "", "--TestAdapter.Endpoints=default" + \
        " --TestAdapter.AdapterId=TestAdapter")

#
# Test client/server with on demand activation.
#
IceGridAdmin.iceGridTest(testdir, name, "simple_server.xml", "--with-deploy")
sys.exit(0)
